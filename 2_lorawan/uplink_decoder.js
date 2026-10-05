function decodeUplink(input) {
  var text = "";

  for (var i = 0; i < input.bytes.length; i++) {
    text += String.fromCharCode(input.bytes[i]);
  }

  try {
    return {
      data: JSON.parse(text),
      warnings: [],
      errors: []
    };
  } catch (err) {
    return {
      data: { raw: text },
      warnings: [],
      errors: ["Invalid JSON payload"]
    };
  }
}
