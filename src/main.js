import watermark_Module from './watermark.js';
import { md5 } from './md5.min.js';
function _isValidFormat(input) {
    const pattern = /^(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})$/;
    return pattern.test(input);
}

var MWM_obj;
var core_is_init = false;
var miaoWM = {
    init: function(){
        MWM_obj = watermark_Module();
        setTimeout(function(){
            core_is_init = true;
            console.log('\n' + ' %c MiaoWM ' + MWM_obj.version + ' v' + MWM_obj.opencv_version + ' %c https://github.com/James18178/MiaoWM.js ' + '\n', 'color: #03a9f4; background: #030307; padding:5px 0;', 'background: #03a9f4; padding:5px 0;');
        },100);
    },
    setFile: function(file_data){
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        var random = md5(Math.random().toString());
        MWM_obj.FS.writeFile(`${random}.png`, new Int8Array(file_data), {flags: 'w', encoding: 'binary'});
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
    simpleWatermark: function(FID, text, color) {
        if (_isValidFormat(color) == false) {
            throw new Error("Color Format Error! Like: 255,255,255");
        }
        if (FID == "") {
            throw new Error("watermark can't be null");
        }
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        try {
            return MWM_obj.simple_watermark(`${FID}.png`, text, color);
        } catch (e) {
            throw new Error("Unable to process image file");
        }
    },
    fullScreenWatermark: function(FID, text, color) {
        if (_isValidFormat(color) == false) {
            throw new Error("Color Format Error! Like: 255,255,255");
        }
        if (FID == "") {
            throw new Error("watermark can't be null");
        }
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        try {
            return MWM_obj.full_screen_watermark(`${FID}.png`, text, color);
        } catch (e) {
            throw new Error("Unable to process image file");
        }
    },
    edgeWatermark: function(FID, text, color, gain) {
        if (_isValidFormat(color) == false) {
            throw new Error("Color Format Error! Like: 255,255,255");
        }
        if (gain > 1) {
            throw new Error("watermark gain can't be greater than 1");
        }
        if (FID == "") {
            throw new Error("watermark can't be null");
        }
        if (core_is_init == false) {
            throw new Error("watermark core is't init");
        }
        try {
            return MWM_obj.edge_watermark(`${FID}.png`, text, color ,gain);
        } catch (e) {
            throw new Error("Unable to process image file");
        }
    },
};

export { miaoWM };
