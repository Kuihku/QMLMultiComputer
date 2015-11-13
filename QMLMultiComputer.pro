TEMPLATE = subdirs
CONFIG += ORDERED
SUBDIRS = \
    Communication \
    ApplicationServer \
    QMLRunner \
    Test

mkpath(Common)
mkpath(Libs)

