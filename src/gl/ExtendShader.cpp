#include "gl/ExtendShader.h"
#include <QFile>
#include <QTextStream>
#include "XC.h"

namespace gl
{

ExtendShader::ExtendShader()
    : mOriginalCode()
    , mVertexCode()
    , mFragmentCode()
    , mVariation()
    , mLog()
{
}

bool ExtendShader::openFromFile(const QString& aFilePath)
{
    mVariation.clear();
    QFile file(aFilePath);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        mOriginalCode = in.readAll();
        return true;
    }
    mLog = file.errorString() + "\n" + aFilePath;
    return false;
}

void ExtendShader::openFromText(const QString& aText)
{
    mVariation.clear();
    mOriginalCode = aText;
    mLog = "";
}

void ExtendShader::setVariationValue(const QString& aName, const QString& aValue)
{
    VariationUnit unit = { aName, aValue };
    mVariation.push_back(unit);
}

bool ExtendShader::resolveVariation()
{
    QTextStream in(&mOriginalCode);
    QTextStream out(&mVertexCode);

    mLog = "";
    bool isSuccess = true;

    while (1)
    {
        QString line = in.readLine();
        if (line.isNull()) break;

        {
            QRegExp vtxReg("^#begin_vertex_shader(\\s+|$)");
            QRegExp frgReg("^#begin_fragment_shader(\\s+|$)");
            if (vtxReg.indexIn(line) != -1)
            {
                out.setString(&mVertexCode);
                continue;
            }
            if (frgReg.indexIn(line) != -1)
            {
                out.setString(&mFragmentCode);
                continue;
            }
        }

        QRegExp lineReg("^#variation\\s+(.*)$");
        if (lineReg.exactMatch(line))
        {
            QString nameValue = lineReg.cap(1);
            //XC_REPORT() << "nameValue" << nameValue;
            QRegExp nameReg("^(\\S+)");
            if (nameReg.indexIn(nameValue) != -1)
            {
                QString name = nameReg.cap(1);
                //XC_REPORT() << "name" << name;

                bool find = false;
                for (std::vector<VariationUnit>::iterator itr = mVariation.begin(); itr != mVariation.end(); ++itr)
                {
                    if (name == itr->name)
                    {
                        line = "#define " + itr->name + " " + itr->value;
                        find = true;
                        break;
                    }
                }
                if (!find)
                {
                    line = "#define " + nameValue;
                }
            }
            else
            {
                mLog += "Invalid variation format:" + line + "\n";
                isSuccess = false;
            }
        }
        out << line << "\n";
    }

    return isSuccess;
}

} // namespace gl
