// webpack.config.js
const path = require('path');

module.exports = {
  mode: 'production',
  entry: './src/main.js',
  output: {
    filename: 'miaoWM.worker.js',
    path: path.resolve(__dirname, 'dist'),
    globalObject: 'this'
  },
  resolve: {
    fallback: {
      "crypto": false,
      "buffer": false,
      "fs": false,
      "path": false
    }
  }
};
