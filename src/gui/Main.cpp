#include "gui/MSVCMemoryLeakDebugger.h" // first of all
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QScopedPointer>
#include "XC.h"
#include "gl/Global.h"
#include "ctrl/System.h"
#include "gui/MainWindow.h"
#include "gui/GUIResources.h"
#include "gui/MSVCBackTracer.h"
#include "gui/LocaleDecider.h"


#if defined(USE_MSVC_MEMORYLEAK_DEBUG)
extern MemoryRegister gMemoryRegister;
#endif // USE_MSVC_MEMORYLEAK_DEBUG

#if defined(USE_MSVC_BACKTRACE)
extern BackTracer gBackTracer;
#endif // USE_MSVC_BACKTRACE

class AEAssertHandler : public XCAssertHandler
{
public:
    virtual void failure() const
    {
#if defined(USE_MSVC_BACKTRACE)
        gBackTracer.dumpCurrent();
#endif // USE_MSVC_BACKTRACE
    }
};

class AEErrorHandler : public XCErrorHandler
{
public:
    virtual void critical(
            const QString& aText, const QString& aInfo,
            const QString& aDetail) const
    {
        XC_REPORT() << aText << "\n" << aInfo << "\n" << aDetail;
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Fatal Error");

        msgBox.setText(aText);
        msgBox.setInformativeText(aInfo);
        if (!aDetail.isEmpty())
        {
            msgBox.setDetailedText(aDetail);
        }

        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();

        std::exit(EXIT_FAILURE);
    }
};

XCAssertHandler* gXCAssertHandler = nullptr;
XCErrorHandler* gXCErrorHandler = nullptr;
static AEAssertHandler sAEAssertHandler;

#if defined(USE_MSVC_BACKTRACE)
#define TRY_ACTION_WITH_EXCEPT(action)                           \
    __try{ action; }                                             \
    __except(EXCEPTION_EXECUTE_HANDLER)                          \
        { qDebug("exception occurred.(%x)", GetExceptionCode()); \
          gBackTracer.dumpCurrent(); std::abort(); }
#else
#define TRY_ACTION_WITH_EXCEPT(action) action
#endif // USE_MSVC_BACKTRACE

int entryPoint(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    gXCAssertHandler = &sAEAssertHandler;

#if defined(USE_MSVC_MEMORYLEAK_DEBUG)
    _CrtSetAllocHook(myAllocHook);
#endif // USE_MSVC_MEMORYLEAK_DEBUG

    int result = 0;
    TRY_ACTION_WITH_EXCEPT(result = entryPoint(argc, argv));

#if defined(USE_MSVC_MEMORYLEAK_DEBUG)
    gMemoryRegister.dumpAll();
    gMemoryRegister.final();
#endif // USE_MSVC_MEMORYLEAK_DEBUG

    return result;
}

int entryPoint(int argc, char *argv[])
{
    int result = 0;

    // create qt application
    QApplication app(argc, argv);
    XC_DEBUG_REPORT() << "exe path =" << app.applicationFilePath();

    // application path
#if defined(Q_OS_MAC)
    const QString appDir = QDir(app.applicationDirPath() + "/../../").absolutePath();
    app.addLibraryPath(app.applicationDirPath() + "/../PlugIns/");
#else
    const QString appDir = app.applicationDirPath();
#endif
    const QString resourceDir(appDir + "/data");
    const QString stdCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    const QString cacheDir = !stdCacheDir.isEmpty() ? stdCacheDir : (appDir + "/cache");

    // initialize current
    QDir::setCurrent(appDir);

    // set fatal error handler
    static AEErrorHandler aeErrorHandler;
    gXCErrorHandler = &aeErrorHandler;

    // set organization and application name for the application setting
    QCoreApplication::setOrganizationName("AnimeEffectsProject");
    QCoreApplication::setApplicationName("AnimeEffects");

    // language
    QScopedPointer<gui::LocaleDecider> locale(new gui::LocaleDecider());
    if (locale->translator())
    {
        app.installTranslator(locale->translator());
    }

    {
        // load constant gui resources
        QScopedPointer<gui::GUIResources> resources(
                    new gui::GUIResources(resourceDir));

        // create system logic core
        QScopedPointer<ctrl::System> system(
                    new ctrl::System(resourceDir, cacheDir));

        // create main window
        QScopedPointer<gui::MainWindow> mainWindow(
                    new gui::MainWindow(*system, *resources, locale->localeParam()));

        qDebug() << "show main window";
        // show main window
        mainWindow->showWithSettings();

#if !defined(QT_NO_DEBUG)
        qDebug() << "test new project";
        const QString testPath = resourceDir + "/sample.psd";
        mainWindow->testNewProject(testPath);
#endif

        // execute application
        result = app.exec();

        // save settings(window status, etc.)
        mainWindow->saveCurrentSettings(result);

        // bind gl context for destructors
        gl::Global::makeCurrent();
        qDebug() << "dst main window";
        mainWindow.reset();
        qDebug() << "dst system";
        system.reset();
        qDebug() << "dst resources";
        resources.reset();
        qDebug() << "dst major components";
    }

    return result;
}
