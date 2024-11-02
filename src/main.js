import watermark_Module from './watermark.js';
import md5  from './md5.min.js';
function _isValidFormat(input) {
    const pattern = /^(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})$/;
    return pattern.test(input);
}

var MWM_obj;
var core_is_init = false;
var miaoWM = {
    OnMessage: function( message ) {
        message = message.data;
        var post_data = {};
        switch(message.command){
            case "coreInit":{ // 内核初始化
                miaoWM.init();
                break;
            }
            case "setFile":{ // 设置文件
                post_data = {fileID: miaoWM.setFile(message.data?.fileData, message.data?.isFont)};
                break;
            }
            case "getVersion":{ // 获取内核版本
                post_data = miaoWM.getVersion();
                break;
            }
            case "simpleWatermark":{ // 简单水印
                post_data = {base64Data: miaoWM.simpleWatermark(message.data?.fontID, message.data?.picID, message.data?.text, message.data?.color)};
                break;
            }
            case "fullScreenWatermark": { // 全屏水印
                post_data = {base64Data: miaoWM.fullScreenWatermark(message.data?.fontID, message.data?.picID, message.data?.text, message.data?.color)};
                break;
            }
            case "edgeWatermark": { // 边缘水印
                post_data = {base64Data: miaoWM.edgeWatermark(message.data?.fontID, message.data?.picID, message.data?.text, message.data?.color, message.data?.fontSize)};
                break;
            }
            default: {
                post_data = {message: "command error"};
                break;
            }
        }
        self.postMessage(post_data);
    },
    init: function(){
        MWM_obj = watermark_Module();
        setTimeout(function(){
            core_is_init = true;
            console.log('\n' + ' %c MiaoWM ' + MWM_obj.version + ' v' + MWM_obj.opencv_version + ' %c https://github.com/James18178/MiaoWM.js ' + '\n', 'color: #03a9f4; background: #030307; padding:5px 0;', 'background: #03a9f4; padding:5px 0;');
        },500);
    },
    setFile: function(file_data, isFont){
        var file_suffix = "png";
        if (isFont) file_suffix = "ttf";
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        var random = md5(Math.random().toString());
        MWM_obj.FS.writeFile(`${random}.${file_suffix}`, new Int8Array(file_data), {flags: 'w', encoding: 'binary'});
        return random;
    },
    getVersion: function() {
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        return {
            "coreVersion": MWM_obj.version,
            "openCVVersion": MWM_obj.opencv_version,
        };
    },
    simpleWatermark: function(FID, PID, text, color) {
        if (_isValidFormat(color) == false) {
            throw new Error("Color Format Error! Like: 255,255,255");
        }
        if (PID == "") {
            throw new Error("original image can't be null");
        }
        if (FID == "") {
            throw new Error("font can't be null");
        }
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        try {
            return MWM_obj.simple_watermark(`${FID}.ttf`, `${PID}.png`, text, color);
        } catch (e) {
            throw new Error("Unable to process image file");
        }
    },
    fullScreenWatermark: function(FID, PID, text, color) {
        if (_isValidFormat(color) == false) {
            throw new Error("Color Format Error! Like: 255,255,255");
        }
        if (PID == "") {
            throw new Error("original image can't be null");
        }
        if (FID == "") {
            throw new Error("font can't be null");
        }
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        try {
            return MWM_obj.full_screen_watermark(`${FID}.ttf`, `${PID}.png`, text, color);
        } catch (e) {
            throw new Error("Unable to process image file");
        }
    },
    edgeWatermark: function(FID, PID, text, color, gain, fontSize) {
        if (_isValidFormat(color) == false) {
            throw new Error("Color Format Error! Like: 255,255,255");
        }
        if (gain > 1) {
            throw new Error("watermark gain can't be greater than 1");
        }
        if (!(fontSize > 0 && fontSize < 100)) {
            throw new Error("watermark fontSize can't be greater than 100 or less than 0");
        }
        if (PID == "") {
            throw new Error("original image can't be null");
        }
        if (FID == "") {
            throw new Error("font can't be null");
        }
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        try {
            return MWM_obj.edge_watermark(`${FID}.ttf`, `${PID}.png`, text, color ,gain, fontSize);
        } catch (e) {
            throw new Error("Unable to process image file");
        }
    },
};

// bind message
self.onmessage = miaoWM.OnMessage.bind(this);
