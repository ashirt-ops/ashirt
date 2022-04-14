
add_library (COMPONENTS STATIC
    aspectratio_pixmap_label/aspectratiopixmaplabel.cpp aspectratio_pixmap_label/aspectratiopixmaplabel.h
    aspectratio_pixmap_label/imageview.cpp aspectratio_pixmap_label/imageview.h
    code_editor/codeblockview.cpp code_editor/codeblockview.h
    code_editor/codeeditor.cpp code_editor/codeeditor.h
    custom_keyseq_edit/singlestrokekeysequenceedit.cpp custom_keyseq_edit/singlestrokekeysequenceedit.h
    error_view/errorview.cpp error_view/errorview.h
    evidence_editor/deleteevidenceresponse.h
    evidence_editor/evidenceeditor.cpp evidence_editor/evidenceeditor.h
    evidence_editor/saveevidenceresponse.h
    evidencepreview.cpp evidencepreview.h
    flow_layout/flowlayout.cpp flow_layout/flowlayout.h
    loading/qprogressindicator.cpp loading/qprogressindicator.h
    loading_button/loadingbutton.cpp loading_button/loadingbutton.h
    tagging/tag_cache/tagcache.cpp tagging/tag_cache/tagcache.h
    tagging/tag_cache/tagcacheitem.cpp tagging/tag_cache/tagcacheitem.h
    tagging/tageditor.cpp tagging/tageditor.h
    tagging/tagginglineediteventfilter.h
    tagging/tagview.cpp tagging/tagview.h
    tagging/tagwidget.cpp tagging/tagwidget.h
)

add_library(ASHIRT::COMPONENTS ALIAS COMPONENTS)

target_include_directories (COMPONENTS
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries ( COMPONENTS PUBLIC
    Qt::Widgets
    Qt::Network
    ASHIRT::DB
)
