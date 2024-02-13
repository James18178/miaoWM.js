// webpack.config.js
const path = require('path');

module.exports = {
  mode: 'production',
  entry: './src/main.js',
  output: {
    filename: 'miaoWM.js',
    path: path.resolve(__dirname, 'dist'),
    libraryTarget: 'commonjs',
    umdNamedDefine: true,
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
