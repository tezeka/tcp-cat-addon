const test = require('brittle')
const { tcpCat } = require('.')

test('tcpCat fetches data over TCP', async (t) => {
  const response = await tcpCat(
    '1.1.1.1',
    80,
    'GET / HTTP/1.1\r\nHost: cloudflare.com\r\nConnection: close\r\n\r\n'
  )
  t.ok(Buffer.isBuffer(response))
  t.ok(response.length > 0)
})
