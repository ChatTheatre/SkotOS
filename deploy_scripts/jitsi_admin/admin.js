// This is intended to be the SkotOS XMPP admin bot.
// It needs to connect to the server to hold chatrooms open.
// It should also be able to kick users approximately on demand (eventually.)

// Puppeteer code based on https://gist.github.com/saghul/179feba3df9f12ddf316decd0181b03e
const http = require('http')
const path = require('path')
const puppeteer = require('puppeteer')

// Possible way to capture video:
// sudo apt-get install chromium-browser xvfb
// Then use xvfb as output device for Chrome (see: https://github.com/r-oung/jitsi-headless)

async function main(room, baseUrl) {
  const chromeArgs = [
    // Disable sandboxing, gives an error on Linux and supposedly breaks fake audio capture
    '--no-sandbox',
    '--disable-extensions',
    '--disable-setuid-sandbox',
    // Automatically give permission to use media devices
    '--use-fake-ui-for-media-stream',
    // feeds a test pattern to getUserMedia() instead of live camera input
    '--use-fake-device-for-media-stream',
    // Silence all output, just in case
    '--alsa-output-device=plug:null',
    // Performance from https://stackoverflow.com/a/58589026/684353
    '--disable-dev-shm-usage',
    '--disable-accelerated-2d-canvas',
    '--no-first-run',
    '--no-zygote',
    '--single-process',
    '--disable-gpu',
  ]

  const browser = await puppeteer.launch({
    args: chromeArgs,
    handleSIGINT: false,
  })

  const page = await browser.newPage()
  page.on('console', (msg) => console.log('CONSOLE:', msg.text()))

  const meetArgs = [
    // Disable receiving of video
    'config.channelLastN=0',
    // Mute our audio
    'config.startWithAudioMuted=true',
    // Don't send video
    'config.startWithVideoMuted=true',
    // No pre-join page
    'config.prejoinPageEnabled=false',
    // Don't use simulcast to save resources on the sender (our) side
    'config.disableSimulcast=true',
    // No need to process audio levels
    'config.disableAudioLevels=true',
    // Disable P2P mode due to a bug in Jitsi Meet
    'config.p2p.enabled=false',
    // Is this useful? Does it even work?
    'config.startSilent=true',
  ]
  const url = `${baseUrl}/${room}#${meetArgs.join('&')}`
  console.log(`Loading ${url}`)

  await page.goto(url)

  // Set some friendly display name
  await page.evaluate('APP.conference.changeLocalDisplayName("Fake Client");')

  console.log('Running...')

  return async () => {
    await page.evaluate('APP.conference.hangup();')
    await page.close()
    await browser.close()
    console.log('Connection to room ' + room + ' stopped.')
  }
}

const jitsiDomain = process.env.JITSI_DOMAIN || 'https://meet.testing-14.madrubyscience.com'

let close = undefined;

setTimeout(function () {
    const room = "RWOTTest";
    close = await main(room, jitsiDomain);
});

// Shut it down...
setTimeout(function () { await close() }, 5000);

// Manual handling on SIGINT to gracefully hangup and exit
process.on('SIGINT', async () => {
  if (close) {
    console.log('Exiting...')
    await close()
    console.log('Done!')
  }
  process.exit()
});
