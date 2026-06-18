const binding = require('./binding')

exports.tcpCat = async function tcpCat(host, port, request) {
  const result = await binding.tcpCat(host, port, request)

  return Buffer.from(result)
}
