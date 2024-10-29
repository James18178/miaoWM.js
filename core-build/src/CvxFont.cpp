#include "cvxFont.h"
#include <cassert>
#include <clocale>
#include <utility>
#include <sstream>
#include <cstdlib>
 
cvx::CvxFont::CvxFont(const cv::String& fontType)
{
    assert(!fontType.empty());
    m_error = FT_Init_FreeType(&m_library);
    if (m_error){
        std::cerr << "library initial error!" << std::endl;
        return;
    }
    m_error = FT_New_Face(m_library, fontType.c_str(), 0, &m_face);
    if (m_error == FT_Err_Unknown_File_Format){
        std::cerr << "unsupported font format!" << std::endl;
        return;
    }
    else if (m_error){
        std::cerr << " can not open font files" << std::endl;
        return;
    }
    // use default parameters
    m_font = new FontProperty;
    initFont();
    setlocale(LC_ALL, "");
}
 
// release freetype resource
cvx::CvxFont::~CvxFont()
{
    delete m_font;
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_library);
}
 
void cvx::CvxFont::setFontSize(const int fontSize)
{
    m_font->fontSize = fontSize;
    FT_Set_Pixel_Sizes(m_face, fontSize, 0);
}
 
// initial font
void cvx::CvxFont::initFont()
{
    setFontSize(16);
    setSpaceRatio(0.5);
    setFontRatio(0);
    setRotateAngle(0);
    setDiaphaneity(1);
    setUnderline(false);
    setVertical(false);
    // set font
    FT_Set_Pixel_Sizes(m_face, getFontSize(), 0);
}
 
int my_mbtowc(wchar_t* dest, const char* src, size_t n)
{
    if (n == 0 || src == nullptr)
    {
        return 0;
    }
 
    uint8_t firstByte = static_cast<uint8_t>(*src);
    int numBytes = 0;
 
    if (firstByte <= 0x7F)
    {
        if (dest != nullptr)
        {
            *dest = static_cast<wchar_t>(firstByte);
        }
        numBytes = 1;
    }
    else if (firstByte >= 0xC2 && firstByte <= 0xDF)
    {
        if (n >= 2 && (static_cast<uint8_t>(src[1]) & 0xC0) == 0x80)
        {
            if (dest != nullptr)
            {
                *dest = ((static_cast<wchar_t>(firstByte) & 0x1F) << 6) |
                        (static_cast<wchar_t>(src[1]) & 0x3F);
            }
            numBytes = 2;
        }
        else
        {
            return -1;
        }
    }
    else if (firstByte >= 0xE0 && firstByte <= 0xEF)
    {
        if (n >= 3 && (static_cast<uint8_t>(src[1]) & 0xC0) == 0x80 &&
            (static_cast<uint8_t>(src[2]) & 0xC0) == 0x80)
        {
            if (dest != nullptr)
            {
                *dest = ((static_cast<wchar_t>(firstByte) & 0x0F) << 12) |
                        ((static_cast<wchar_t>(src[1]) & 0x3F) << 6) |
                        (static_cast<wchar_t>(src[2]) & 0x3F);
            }
            numBytes = 3;
        }
        else
        {
            return -1;
        }
    }
    else if (firstByte >= 0xF0 && firstByte <= 0xF4)
    {
        if (n >= 4 && (static_cast<uint8_t>(src[1]) & 0xC0) == 0x80 &&
            (static_cast<uint8_t>(src[2]) & 0xC0) == 0x80 &&
            (static_cast<uint8_t>(src[3]) & 0xC0) == 0x80)
        {
            if (dest != nullptr)
            {
                *dest = ((static_cast<wchar_t>(firstByte) & 0x07) << 18) |
                        ((static_cast<wchar_t>(src[1]) & 0x3F) << 12) |
                        ((static_cast<wchar_t>(src[2]) & 0x3F) << 6) |
                        (static_cast<wchar_t>(src[3]) & 0x3F);
            }
            numBytes = 4;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
 
    return numBytes;
}
 
void cvx::CvxFont::rotateFont(double angle) {
    angle = (angle / 360) * 3.14159 * 2;
    /* set up matrix */
    m_matrix.xx = static_cast<FT_Fixed>(cos(angle) * 0x10000L);
    m_matrix.xy = static_cast<FT_Fixed>(-sin(angle) * 0x10000L);
    m_matrix.yx = static_cast<FT_Fixed>(sin(angle) * 0x10000L);
    m_matrix.yy = static_cast<FT_Fixed>(cos(angle) * 0x10000L);
 
    FT_Set_Transform(m_face, &m_matrix, nullptr);
}
 
void cvx::CvxFont::putTextStr(cv::Mat& img, const cv::String& text, cv::Point pos, const cv::Scalar& color)
{
    CV_Assert(!img.empty());
    CV_Assert(!text.empty());
 
    int xStart = pos.x;
    int yStart = pos.y;
    m_maxDiffHeight = 0;
    const char* ptr = text.c_str();
    std::mbtowc(nullptr, nullptr, 0); // reset the conversion state
    const char* end = ptr + std::strlen(ptr);
    int ret;
    wchar_t wc;
    for (wchar_t wc; (ret = my_mbtowc(&wc, ptr, end - ptr)) > 0; ptr += ret) {
        putWChar(img, (wc & 0xffffffff), pos, color);
    }
 
    int xEnd = pos.x;
    int yEnd = pos.y;
    if (getUnderline()) {
        if (getVertical()) {
            cv::line(img, cv::Point(xStart + m_maxDiffHeight, yStart), cv::Point(xStart + m_maxDiffHeight, yEnd), color, 2);
        }
        else {
            cv::line(img, cv::Point(xStart, yStart + m_maxDiffHeight), cv::Point(xEnd, yStart + m_maxDiffHeight), color, 2);
        }
    }
 
}
 
void cvx::CvxFont::putWChar(cv::Mat& img, uint32_t wc, cv::Point& pos, const cv::Scalar& color)
{
    rotateFont(getAngle());
    const auto vertical = getVertical();
    const auto size = getFontSize();
 
    // Converting a Character Code Into a Glyph Index
    FT_UInt glyph_index = FT_Get_Char_Index(m_face, wc);
    FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
    FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_MONO);
 
    FT_GlyphSlot slot = m_face->glyph;
    FT_Bitmap bitmap = slot->bitmap;
    bool isSpace = wc == ' ';
 
    // get rows and cols of current wide char
    auto rows = bitmap.rows;
    auto cols = bitmap.width;
 
    cv::Point gPos = pos;
    //gPos.y += m_font->fontSize;
    if (vertical)
    {
        gPos.x += (slot->metrics.vertBearingX >> 6);
        gPos.y += (slot->metrics.vertBearingY >> 6);
        m_maxDiffHeight = std::max(m_maxDiffHeight, rows - (slot->metrics.vertBearingY >> 6));
    }
    else
    {
        gPos.x += (slot->metrics.horiBearingX >> 6);
        gPos.y -= (slot->metrics.horiBearingY >> 6);
        m_maxDiffHeight = std::max(m_maxDiffHeight, rows - (slot->metrics.horiBearingY >> 6));
    }
 
    // https://stackoverflow.com/questions/52254639/how-to-access-pixels-state-in-monochrome-bitmap-using-freetype2
    for (auto i = 0; i < rows; ++i)
    {
        for (auto j = 0; j < cols; ++j)
        {
            int off = i * slot->bitmap.pitch + j / 8;
 
            if (slot->bitmap.buffer[off] & (0x80 >> (j % 8)))
            {
                const auto r = gPos.y + i; //vertical ? pos.y + i : pos.y + i + (size - rows); // to make align to bottom
                const auto c = gPos.x + j;
 
                if (r >= 0 && r < img.rows && c >= 0 && c < img.cols)
                {
                	if (color.channels == 3)
                	{
		                cv::Vec3b scalar = img.at<cv::Vec3b>(cv::Point(c, r));
		                // merge set color with origin color
		                double p = getDiaphaneity();
		                for (int k = 0; k < 3; ++k)
		                {
		                    scalar.val[k] = static_cast<uchar>(scalar.val[k] * (1 - p) + color.val[k] * p);
		                }
 
		                img.at<cv::Vec3b>(cv::Point(c, r)) = cv::Vec3b(scalar[0], scalar[1], scalar[2]);                	
                	}
                	else if (color.channels == 4)
		            {
		                cv::Vec4b scalar = img.at<cv::Vec4b>(cv::Point(c, r));
		                // merge set color with origin color
		                double p = getDiaphaneity();
		                for (int k = 0; k < 4; ++k)
		                {
		                    scalar.val[k] = static_cast<uchar>(scalar.val[k] * (1 - p) + color.val[k] * p);
		                }
 
		                img.at<cv::Vec4b>(cv::Point(c, r)) = cv::Vec4b(scalar[0], scalar[1], scalar[2], scalar[3]);    
		            }
                }
            }
        }
    }
    // modify position to next character
    const auto space = static_cast<int>(size * getSpaceRatio());
    const auto sep = static_cast<int>(size * getFontRatio());
    // vertical string or not, default not vertical
    if (vertical){
        const auto moveX = (static_cast<int>(getAngle()) == 0) ?  (slot->metrics.vertAdvance >> 6) : rows + 1;
        pos.y += isSpace ? space : moveX + sep;
    }else{
        const auto moveY = (static_cast<int>(getAngle()) == 0) ? (slot->metrics.horiAdvance >> 6) : cols + 1;
        pos.x += isSpace ? space : moveY + sep;
    }
}
 
void cvx::putText(cv::Mat& img, const std::string& text, cv::Point pos, cvx::CvxFont& fontFace, int fontSize, const cv::Scalar& color) {
    fontFace.setFontSize(fontSize);
    fontFace.putTextStr(img, text, std::move(pos), color);
    fontFace.initFont();
}