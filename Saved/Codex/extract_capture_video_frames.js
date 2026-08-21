const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright");

const videoPath = "C:/Users/nos36/Videos/화면 녹화/화면 녹화 중 2026-07-08 110349.mp4";
const outputDir = "C:/Unreal/PalWorld/Saved/Codex/VideoFrames";
const chromePath = "C:/Program Files/Google/Chrome/Application/chrome.exe";

function fileUrl(p) {
  return "file:///" + p.replace(/\\/g, "/").replace(/ /g, "%20");
}

(async () => {
  fs.mkdirSync(outputDir, { recursive: true });

  const htmlPath = path.join(outputDir, "capture_video_probe.html");
  fs.writeFileSync(
    htmlPath,
    `<!doctype html>
<html>
<body style="margin:0;background:#111;display:flex;align-items:center;justify-content:center;height:100vh;overflow:hidden">
<video id="v" src="${fileUrl(videoPath)}" style="max-width:100vw;max-height:100vh" muted></video>
<script>
const v = document.getElementById('v');
window.seekVideo = async (t) => {
  await new Promise((resolve) => {
    if (v.readyState >= 1) resolve();
    else v.addEventListener('loadedmetadata', resolve, { once: true });
  });
  v.currentTime = Math.min(Math.max(0, t), v.duration || t);
  await new Promise((resolve) => v.addEventListener('seeked', resolve, { once: true }));
  return { duration: v.duration, currentTime: v.currentTime, width: v.videoWidth, height: v.videoHeight };
};
</script>
</body>
</html>`,
    "utf8"
  );

  const browser = await chromium.launch({
    headless: true,
    executablePath: chromePath,
    args: ["--autoplay-policy=no-user-gesture-required"],
  });
  const page = await browser.newPage({ viewport: { width: 1280, height: 720 }, deviceScaleFactor: 1 });
  await page.goto(fileUrl(htmlPath));

  const meta = await page.evaluate(() => window.seekVideo(0));
  const duration = meta.duration || 0;
  const times = [];
  if (duration > 0) {
    const count = 12;
    for (let i = 0; i < count; ++i) {
      times.push((duration * i) / (count - 1));
    }
  } else {
    times.push(0, 0.5, 1, 1.5, 2, 2.5, 3);
  }

  const written = [];
  for (let i = 0; i < times.length; ++i) {
    const info = await page.evaluate((t) => window.seekVideo(t), times[i]);
    const out = path.join(outputDir, `capture_frame_${String(i).padStart(2, "0")}.png`);
    await page.screenshot({ path: out });
    written.push({ out, time: info.currentTime, duration: info.duration, size: `${info.width}x${info.height}` });
  }

  await browser.close();
  console.log(JSON.stringify({ duration, frames: written }, null, 2));
})().catch((err) => {
  console.error(err.stack || err);
  process.exit(1);
});
