#ifndef APPDATA_H
#define APPDATA_H

#include <QStringList>

class AppData {
public:
    static AppData& instance() {
        static AppData _instance;
        return _instance;
    }
    QStringList TagsSelected;
    QStringList lastTagsSelected;
    bool ignoreCacheErrors = false;
private:
    AppData() {}
};

#endif // APPDATA_H
