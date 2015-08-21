TEMPLATE = lib
TARGET = fbreader
CONFIG += staticlib object_parallel_to_source link_pkgconfig
PKGCONFIG += glib-2.0

!include(../common.pri)

# Directories
FRIBIDI_DIR = $$_PRO_FILE_PWD_/../fribidi
LINEBREAK_DIR = $$_PRO_FILE_PWD_/../linebreak
FBREADER_DIR = fbreader

DEFINES += \
  FBREADER_THREAD_LOCAL_ZLFILE_PLAIN_STREAM_CACHE=1 \
  FBREADER_DISABLE_BOOKS_DB=1

# Core
CORE = $$FBREADER_DIR/zlibrary/core
CORE_SRC = $$CORE/src

INCLUDEPATH += $$CORE/include

SOURCES += \
  $$CORE_SRC/encoding/MyEncodingConverter.cpp \
  $$CORE_SRC/encoding/EncodingCollectionReader.cpp \
  $$CORE_SRC/encoding/DummyEncodingConverter.cpp \
  $$CORE_SRC/encoding/ZLEncodingSet.cpp \
  $$CORE_SRC/encoding/ZLEncodingConverter.cpp \
  $$CORE_SRC/encoding/ZLEncodingCollection.cpp \
  $$CORE_SRC/typeId/ZLTypeId.cpp \
  $$CORE_SRC/language/ZLLanguageDetector.cpp \
  $$CORE_SRC/language/ZLStatistics.cpp \
  $$CORE_SRC/language/ZLStatisticsGenerator.cpp \
  $$CORE_SRC/language/ZLStatisticsXMLReader.cpp \
  $$CORE_SRC/language/ZLStatisticsXMLWriter.cpp \
  $$CORE_SRC/language/ZLLanguageList.cpp \
  $$CORE_SRC/language/ZLCharSequence.cpp \
  $$CORE_SRC/language/ZLStatisticsItem.cpp \
  $$CORE_SRC/language/ZLLanguageMatcher.cpp \
  $$CORE_SRC/options/ZLConfig.cpp \
  $$CORE_SRC/options/ZLOptions.cpp \
  $$CORE_SRC/options/ZLCategoryKey.cpp \
  $$CORE_SRC/runnable/ZLRunnable.cpp \
  $$CORE_SRC/runnable/ZLExecutionData.cpp \
  $$CORE_SRC/application/ZLApplicationBase.cpp \
  $$CORE_SRC/application/ZLKeyBindings.cpp \
  $$CORE_SRC/util/ZLUnicodeUtil.cpp \
  $$CORE_SRC/util/ZLKeyUtil.cpp \
  $$CORE_SRC/util/ZLUserData.cpp \
  $$CORE_SRC/util/ZLFileUtil.cpp \
  $$CORE_SRC/util/ZLSearchUtil.cpp \
  $$CORE_SRC/util/ZLStringUtil.cpp \
  $$CORE_SRC/util/ZLLanguageUtil.cpp \
  $$CORE_SRC/dialogs/ZLDialogManager.cpp \
  $$CORE_SRC/dialogs/ZLDialog.cpp \
  $$CORE_SRC/dialogs/ZLOptionEntry.cpp \
  $$CORE_SRC/dialogs/ZLOptionsDialog.cpp \
  $$CORE_SRC/dialogs/ZLProgressDialog.cpp \
  $$CORE_SRC/dialogs/ZLOptionView.cpp \
  $$CORE_SRC/dialogs/ZLDialogContent.cpp \
  $$CORE_SRC/dialogs/ZLDialogContentBuilder.cpp \
  $$CORE_SRC/dialogs/ZLOpenFileDialog.cpp \
  $$CORE_SRC/time/ZLTimeManager.cpp \
  $$CORE_SRC/time/ZLTime.cpp \
  $$CORE_SRC/logger/ZLLogger.cpp \
  $$CORE_SRC/optionEntries/ZLSimpleOptionEntry.cpp \
  $$CORE_SRC/xml/ZLXMLWriter.cpp \
  $$CORE_SRC/xml/ZLXMLReader.cpp \
  $$CORE_SRC/xml/expat/ZLXMLReaderInternal.cpp \
  $$CORE_SRC/view/ZLPaintContext.cpp \
  $$CORE_SRC/view/ZLView.cpp \
  $$CORE_SRC/view/ZLMirroredPaintContext.cpp \
  $$CORE_SRC/filesystem/tar/ZLTar.cpp \
  $$CORE_SRC/filesystem/ZLInputStreamDecorator.cpp \
  $$CORE_SRC/filesystem/ZLFile.cpp \
  $$CORE_SRC/filesystem/ZLDir.cpp \
  $$CORE_SRC/filesystem/ZLFSManager.cpp \
  $$CORE_SRC/filesystem/zip/ZLZipInputStream.cpp \
  $$CORE_SRC/filesystem/zip/ZLZipDir.cpp \
  $$CORE_SRC/filesystem/zip/ZLZipEntryCache.cpp \
  $$CORE_SRC/filesystem/zip/ZLZipHeader.cpp \
  $$CORE_SRC/filesystem/zip/ZLZDecompressor.cpp \
  $$CORE_SRC/filesystem/zip/ZLGzipInputStream.cpp \
  $$CORE_SRC/filesystem/bzip2/ZLBzip2InputStream.cpp \
  $$CORE_SRC/resources/ZLResource.cpp \
  $$CORE_SRC/image/ZLBase64EncodedImage.cpp \
  $$CORE_SRC/image/ZLNetworkImage.cpp \
  $$CORE_SRC/image/ZLImage.cpp \
  $$CORE_SRC/image/ZLFileImage.cpp \
  $$CORE_SRC/image/ZLStreamImage.cpp \
  $$CORE_SRC/image/ZLImageManager.cpp \
  $$CORE_SRC/unix/iconv/IConvEncodingConverter.cpp \
  $$CORE_SRC/unix/time/ZLUnixTime.cpp \
  $$CORE_SRC/unix/filesystem/ZLUnixFSManager.cpp \
  $$CORE_SRC/unix/filesystem/ZLUnixFileInputStream.cpp \
  $$CORE_SRC/unix/filesystem/ZLUnixFSDir.cpp \
  $$CORE_SRC/unix/filesystem/ZLUnixFileOutputStream.cpp \
  $$CORE_SRC/network/ZLAsynchronousInputStream.cpp \
  $$CORE_SRC/network/ZLPlainAsynchronousInputStream.cpp \
  $$CORE_SRC/constants/ZLXMLNamespace.cpp \
  $$CORE_SRC/constants/ZLMimeType.cpp

HEADERS += \
  $$CORE_SRC/encoding/EncodingCollectionReader.h \
  $$CORE_SRC/encoding/DummyEncodingConverter.h \
  $$CORE_SRC/encoding/ZLEncodingConverterProvider.h \
  $$CORE_SRC/encoding/MyEncodingConverter.h \
  $$CORE_SRC/encoding/ZLEncodingConverter.h \
  $$CORE_SRC/typeId/ZLTypeId.h \
  $$CORE_SRC/language/ZLLanguageList.h \
  $$CORE_SRC/language/ZLStatistics.h \
  $$CORE_SRC/language/ZLStatisticsGenerator.h \
  $$CORE_SRC/language/ZLLanguageMatcher.h \
  $$CORE_SRC/language/ZLLanguageDetector.h \
  $$CORE_SRC/language/ZLStatisticsXMLWriter.h \
  $$CORE_SRC/language/ZLStatisticsItem.h \
  $$CORE_SRC/language/ZLCharSequence.h \
  $$CORE_SRC/language/ZLStatisticsXMLReader.h \
  $$CORE_SRC/options/ZLOptions.h \
  $$CORE_SRC/options/ZLConfig.h \
  $$CORE_SRC/runnable/ZLExecutionData.h \
  $$CORE_SRC/runnable/ZLRunnable.h \
  $$CORE_SRC/application/ZLApplication.h \
  $$CORE_SRC/application/ZLKeyBindings.h \
  $$CORE_SRC/application/ZLApplicationWindow.h \
  $$CORE_SRC/util/ZLStringUtil.h \
  $$CORE_SRC/util/ZLUserData.h \
  $$CORE_SRC/util/ZLBoolean3.h \
  $$CORE_SRC/util/shared_ptr.h \
  $$CORE_SRC/util/ZLFileUtil.h \
  $$CORE_SRC/util/ZLColor.h \
  $$CORE_SRC/util/ZLUnicodeUtil.h \
  $$CORE_SRC/util/ZLSearchUtil.h \
  $$CORE_SRC/util/allocator.h \
  $$CORE_SRC/util/ZLKeyUtil.h \
  $$CORE_SRC/util/ZLLanguageUtil.h \
  $$CORE_SRC/dialogs/ZLOptionsDialog.h \
  $$CORE_SRC/dialogs/ZLDialogManager.h \
  $$CORE_SRC/dialogs/ZLOptionView.h \
  $$CORE_SRC/dialogs/ZLDialog.h \
  $$CORE_SRC/dialogs/ZLProgressDialog.h \
  $$CORE_SRC/dialogs/ZLOptionEntry.h \
  $$CORE_SRC/dialogs/ZLDialogContent.h \
  $$CORE_SRC/dialogs/ZLDialogContentBuilder.h \
  $$CORE_SRC/dialogs/ZLOpenFileDialog.h \
  $$CORE_SRC/library/ZLibrary.h \
  $$CORE_SRC/time/ZLTime.h \
  $$CORE_SRC/time/ZLTimeManager.h \
  $$CORE_SRC/logger/ZLLogger.h \
  $$CORE_SRC/optionEntries/ZLSimpleOptionEntry.h \
  $$CORE_SRC/xml/ZLXMLWriter.h \
  $$CORE_SRC/xml/ZLXMLReader.h \
  $$CORE_SRC/xml/expat/ZLXMLReaderInternal.h \
  $$CORE_SRC/view/ZLPaintContext.h \
  $$CORE_SRC/view/ZLMirroredPaintContext.h \
  $$CORE_SRC/view/ZLViewWidget.h \
  $$CORE_SRC/view/ZLView.h \
  $$CORE_SRC/filesystem/tar/ZLTar.h \
  $$CORE_SRC/filesystem/ZLInputStream.h \
  $$CORE_SRC/filesystem/ZLFSManager.h \
  $$CORE_SRC/filesystem/ZLDir.h \
  $$CORE_SRC/filesystem/ZLFileInfo.h \
  $$CORE_SRC/filesystem/zip/ZLZDecompressor.h \
  $$CORE_SRC/filesystem/zip/ZLZipHeader.h \
  $$CORE_SRC/filesystem/zip/ZLZip.h \
  $$CORE_SRC/filesystem/ZLOutputStream.h \
  $$CORE_SRC/filesystem/ZLFile.h \
  $$CORE_SRC/filesystem/ZLFSDir.h \
  $$CORE_SRC/filesystem/bzip2/ZLBzip2InputStream.h \
  $$CORE_SRC/resources/ZLResource.h \
  $$CORE_SRC/image/ZLImage.h \
  $$CORE_SRC/image/ZLBase64EncodedImage.h \
  $$CORE_SRC/image/ZLFileImage.h \
  $$CORE_SRC/image/ZLStreamImage.h \
  $$CORE_SRC/image/ZLImageManager.h \
  $$CORE_SRC/image/ZLNetworkImage.h \
  $$CORE_SRC/unix/iconv/IConvEncodingConverter.h \
  $$CORE_SRC/unix/library/ZLibraryImplementation.h \
  $$CORE_SRC/unix/time/ZLUnixTime.h \
  $$CORE_SRC/unix/filesystem/ZLUnixFSManager.h \
  $$CORE_SRC/unix/filesystem/ZLUnixFileInputStream.h \
  $$CORE_SRC/unix/filesystem/ZLUnixFileOutputStream.h \
  $$CORE_SRC/unix/filesystem/ZLUnixFSDir.h \
  $$CORE_SRC/network/ZLAsynchronousInputStream.h \
  $$CORE_SRC/network/ZLPlainAsynchronousInputStream.h \
  $$CORE_SRC/constants/ZLMimeType.h \
  $$CORE_SRC/constants/ZLXMLNamespace.h

# Text
TEXT = $$FBREADER_DIR/zlibrary/text
TEXT_SRC = $$TEXT/src

INCLUDEPATH += \
  $$TEXT/include \
  $$LINEBREAK_DIR/include \
  $$FRIBIDI_DIR/include

SOURCES += \
  $$TEXT_SRC/style/ZLTextStyle.cpp \
  $$TEXT_SRC/style/ZLTextDecoratedStyle.cpp \
  $$TEXT_SRC/style/ZLTextStyleCollection.cpp \
  $$TEXT_SRC/model/ZLTextModel.cpp \
  $$TEXT_SRC/model/ZLTextParagraph.cpp \
  $$TEXT_SRC/model/ZLTextRowMemoryAllocator.cpp \
  $$TEXT_SRC/hyphenation/ZLTextTeXHyphenator.cpp \
  $$TEXT_SRC/hyphenation/ZLTextHyphenator.cpp \
  $$TEXT_SRC/hyphenation/ZLTextHyphenationReader.cpp \
  $$TEXT_SRC/view/ZLTextPositionIndicator.cpp \
  $$TEXT_SRC/view/ZLTextSelectionScroller.cpp \
  $$TEXT_SRC/view/ZLTextView_paint.cpp \
  $$TEXT_SRC/view/ZLTextView.cpp \
  $$TEXT_SRC/area/ZLTextArea.cpp \
  $$TEXT_SRC/area/ZLTextArea_processTextLine.cpp \
  $$TEXT_SRC/area/ZLTextArea_drawTextLine.cpp \
  $$TEXT_SRC/area/ZLTextAreaController.cpp \
  $$TEXT_SRC/area/ZLTextParagraphBuilder.cpp \
  $$TEXT_SRC/area/ZLTextParagraphCursor.cpp \
  $$TEXT_SRC/area/ZLTextWord.cpp \
  $$TEXT_SRC/area/ZLTextElement.cpp \
  $$TEXT_SRC/area/ZLTextAreaStyle.cpp \
  $$TEXT_SRC/area/ZLTextArea_drawWord.cpp \
  $$TEXT_SRC/area/ZLTextArea_drawTreeLines.cpp \
  $$TEXT_SRC/area/ZLTextArea_prepareTextLine.cpp \
  $$TEXT_SRC/area/ZLTextSelectionModel.cpp

HEADERS += \
  $$TEXT_SRC/style/ZLTextStyleCollection.h \
  $$TEXT_SRC/style/ZLTextStyle.h \
  $$TEXT_SRC/style/ZLTextDecoratedStyle.h \
  $$TEXT_SRC/model/ZLTextAlignmentType.h \
  $$TEXT_SRC/model/ZLTextFontModifier.h \
  $$TEXT_SRC/model/ZLTextParagraph.h \
  $$TEXT_SRC/model/ZLTextModel.h \
  $$TEXT_SRC/model/ZLTextMark.h \
  $$TEXT_SRC/model/ZLTextRowMemoryAllocator.h \
  $$TEXT_SRC/model/ZLTextKind.h \
  $$TEXT_SRC/hyphenation/ZLTextHyphenator.h \
  $$TEXT_SRC/hyphenation/ZLTextHyphenationReader.h \
  $$TEXT_SRC/hyphenation/ZLTextTeXHyphenator.h \
  $$TEXT_SRC/view/ZLTextView.h \
  $$TEXT_SRC/view/ZLTextPositionIndicatorInfo.h \
  $$TEXT_SRC/view/ZLTextPositionIndicator.h \
  $$TEXT_SRC/view/ZLTextSelectionScroller.h \
  $$TEXT_SRC/area/ZLTextWord.h \
  $$TEXT_SRC/area/ZLTextElement.h \
  $$TEXT_SRC/area/ZLTextRectangle.h \
  $$TEXT_SRC/area/ZLTextAreaStyle.h \
  $$TEXT_SRC/area/ZLTextParagraphBuilder.h \
  $$TEXT_SRC/area/ZLTextAreaController.h \
  $$TEXT_SRC/area/ZLTextLineInfo.h \
  $$TEXT_SRC/area/ZLTextSelectionModel.h \
  $$TEXT_SRC/area/ZLTextParagraphCursor.h \
  $$TEXT_SRC/area/ZLTextArea.h

# UI

UI_SRC = $$FBREADER_DIR/zlibrary/ui/src/qt4

SOURCES += \
  $$UI_SRC/time/ZLQtTime.cpp \
  $$UI_SRC/filesystem/ZLQtFSManager.cpp \
  $$UI_SRC/image/ZLQtImageManager.cpp

HEADERS += \
  $$UI_SRC/time/ZLQtTime.h \
  $$UI_SRC/filesystem/ZLQtFSManager.h \
  $$UI_SRC/image/ZLQtImageManager.h

# FBReader

FBREADER_SRC = $$FBREADER_DIR/fbreader/src

SOURCES += \
  $$FBREADER_SRC/migration/BookInfo.cpp \
  $$FBREADER_SRC/options/FBCategoryKey.cpp \
  $$FBREADER_SRC/fbreader/SearchActions.cpp \
  $$FBREADER_SRC/fbreader/ContentsView.cpp \
  $$FBREADER_SRC/fbreader/BookTextView.cpp \
  $$FBREADER_SRC/library/Book.cpp \
  $$FBREADER_SRC/library/Comparators.cpp \
  $$FBREADER_SRC/library/Library.cpp \
  $$FBREADER_SRC/library/Tag.cpp \
  $$FBREADER_SRC/library/Author.cpp \
  $$FBREADER_SRC/formats/dummy/DummyBookReader.cpp \
  $$FBREADER_SRC/formats/dummy/DummyMetaInfoReader.cpp \
  $$FBREADER_SRC/formats/fb2/FB2MetaInfoReader.cpp \
  $$FBREADER_SRC/formats/fb2/FB2TagManager.cpp \
  $$FBREADER_SRC/formats/fb2/FB2Plugin.cpp \
  $$FBREADER_SRC/formats/fb2/FB2Reader.cpp \
  $$FBREADER_SRC/formats/fb2/FB2BookReader.cpp \
  $$FBREADER_SRC/formats/fb2/FB2CoverReader.cpp \
  $$FBREADER_SRC/formats/openreader/OpenReaderPlugin.cpp \
  $$FBREADER_SRC/formats/openreader/ORDescriptionReader.cpp \
  $$FBREADER_SRC/formats/openreader/ORBookReader.cpp \
  $$FBREADER_SRC/formats/util/TextFormatDetector.cpp \
  $$FBREADER_SRC/formats/util/EntityFilesCollector.cpp \
  $$FBREADER_SRC/formats/util/MiscUtil.cpp \
  $$FBREADER_SRC/formats/util/XMLTextStream.cpp \
  $$FBREADER_SRC/formats/util/MergedStream.cpp \
  $$FBREADER_SRC/formats/FormatPlugin.cpp \
  $$FBREADER_SRC/formats/EncodedTextReader.cpp \
  $$FBREADER_SRC/formats/css/StyleSheetParser.cpp \
  $$FBREADER_SRC/formats/css/StyleSheetTable.cpp \
  $$FBREADER_SRC/formats/tcr/TcrPlugin.cpp \
  $$FBREADER_SRC/formats/tcr/TcrStream.cpp \
  $$FBREADER_SRC/formats/tcr/PPLBookReader.cpp \
  $$FBREADER_SRC/formats/rtf/RtfReaderStream.cpp \
  $$FBREADER_SRC/formats/rtf/RtfBookReader.cpp \
  $$FBREADER_SRC/formats/rtf/RtfReader.cpp \
  $$FBREADER_SRC/formats/rtf/RtfDescriptionReader.cpp \
  $$FBREADER_SRC/formats/rtf/RtfPlugin.cpp \
  $$FBREADER_SRC/formats/rtf/RtfImage.cpp \
  $$FBREADER_SRC/formats/pdb/PmlBookReader.cpp \
  $$FBREADER_SRC/formats/pdb/PalmDocLikePlugin.cpp \
  $$FBREADER_SRC/formats/pdb/HuffDecompressor.cpp \
  $$FBREADER_SRC/formats/pdb/EReaderStream.cpp \
  $$FBREADER_SRC/formats/pdb/MobipocketPlugin.cpp \
  $$FBREADER_SRC/formats/pdb/PluckerBookReader.cpp \
  $$FBREADER_SRC/formats/pdb/PalmDocLikeStream.cpp \
  $$FBREADER_SRC/formats/pdb/PluckerImages.cpp \
  $$FBREADER_SRC/formats/pdb/PdbStream.cpp \
  $$FBREADER_SRC/formats/pdb/PmlReader.cpp \
  $$FBREADER_SRC/formats/pdb/BitReader.cpp \
  $$FBREADER_SRC/formats/pdb/PluckerTextStream.cpp \
  $$FBREADER_SRC/formats/pdb/MobipocketHtmlBookReader.cpp \
  $$FBREADER_SRC/formats/pdb/ZTXTStream.cpp \
  $$FBREADER_SRC/formats/pdb/ZTXTPlugin.cpp \
  $$FBREADER_SRC/formats/pdb/PdbPlugin.cpp \
  $$FBREADER_SRC/formats/pdb/HtmlMetainfoReader.cpp \
  $$FBREADER_SRC/formats/pdb/PdbReader.cpp \
  $$FBREADER_SRC/formats/pdb/PalmDocStream.cpp \
  $$FBREADER_SRC/formats/pdb/PalmDocPlugin.cpp \
  $$FBREADER_SRC/formats/pdb/DocDecompressor.cpp \
  $$FBREADER_SRC/formats/pdb/SimplePdbPlugin.cpp \
  $$FBREADER_SRC/formats/pdb/PluckerPlugin.cpp \
  $$FBREADER_SRC/formats/pdb/EReaderPlugin.cpp \
  $$FBREADER_SRC/formats/xhtml/XHTMLReader.cpp \
  $$FBREADER_SRC/formats/oeb/NCXReader.cpp \
  $$FBREADER_SRC/formats/oeb/OCFContainerReader.cpp \
  $$FBREADER_SRC/formats/oeb/OEBPlugin.cpp \
  $$FBREADER_SRC/formats/oeb/OEBMetaInfoReader.cpp \
  $$FBREADER_SRC/formats/oeb/OEBCoverReader.cpp \
  $$FBREADER_SRC/formats/oeb/OEBTextStream.cpp \
  $$FBREADER_SRC/formats/oeb/OEBBookReader.cpp \
  $$FBREADER_SRC/formats/PluginCollection.cpp \
  $$FBREADER_SRC/formats/html/HtmlPlugin.cpp \
  $$FBREADER_SRC/formats/html/HtmlReaderStream.cpp \
  $$FBREADER_SRC/formats/html/HtmlEntityCollection.cpp \
  $$FBREADER_SRC/formats/html/HtmlBookReader.cpp \
  $$FBREADER_SRC/formats/html/HtmlDescriptionReader.cpp \
  $$FBREADER_SRC/formats/html/HtmlReader.cpp \
  $$FBREADER_SRC/formats/chm/CHMFileImage.cpp \
  $$FBREADER_SRC/formats/chm/HHCReferenceCollector.cpp \
  $$FBREADER_SRC/formats/chm/CHMReferenceCollection.cpp \
  $$FBREADER_SRC/formats/chm/BitStream.cpp \
  $$FBREADER_SRC/formats/chm/HtmlSectionReader.cpp \
  $$FBREADER_SRC/formats/chm/LZXDecompressor.cpp \
  $$FBREADER_SRC/formats/chm/HuffmanDecoder.cpp \
  $$FBREADER_SRC/formats/chm/CHMFile.cpp \
  $$FBREADER_SRC/formats/chm/HHCReader.cpp \
  $$FBREADER_SRC/formats/chm/CHMPlugin.cpp \
  $$FBREADER_SRC/formats/chm/E8Decoder.cpp \
  $$FBREADER_SRC/formats/txt/TxtReader.cpp \
  $$FBREADER_SRC/formats/txt/PlainTextFormat.cpp \
  $$FBREADER_SRC/formats/txt/TxtBookReader.cpp \
  $$FBREADER_SRC/formats/txt/TxtPlugin.cpp \
  $$FBREADER_SRC/bookmodel/BookReader.cpp \
  $$FBREADER_SRC/bookmodel/BookModel.cpp \

HEADERS += \
  $$FBREADER_SRC/migration/BookInfo.h \
  $$FBREADER_SRC/options/FBCategoryKey.h \
  $$FBREADER_SRC/fbreader/FBView.h \
  $$FBREADER_SRC/fbreader/ReadingState.h \
  $$FBREADER_SRC/fbreader/FBReaderActions.h \
  $$FBREADER_SRC/fbreader/ContentsView.h \
  $$FBREADER_SRC/fbreader/FootnoteView.h \
  $$FBREADER_SRC/fbreader/BookTextView.h \
  $$FBREADER_SRC/library/Author.h \
  $$FBREADER_SRC/library/Book.h \
  $$FBREADER_SRC/library/Lists.h \
  $$FBREADER_SRC/library/Library.h \
  $$FBREADER_SRC/library/Tag.h \
  $$FBREADER_SRC/formats/dummy/DummyBookReader.h \
  $$FBREADER_SRC/formats/dummy/DummyMetaInfoReader.h \
  $$FBREADER_SRC/formats/fb2/FB2Plugin.h \
  $$FBREADER_SRC/formats/fb2/FB2MetaInfoReader.h \
  $$FBREADER_SRC/formats/fb2/FB2TagManager.h \
  $$FBREADER_SRC/formats/fb2/FB2BookReader.h \
  $$FBREADER_SRC/formats/fb2/FB2Reader.h \
  $$FBREADER_SRC/formats/fb2/FB2CoverReader.h \
  $$FBREADER_SRC/formats/openreader/OpenReaderPlugin.h \
  $$FBREADER_SRC/formats/openreader/ORDescriptionReader.h \
  $$FBREADER_SRC/formats/openreader/ORBookReader.h \
  $$FBREADER_SRC/formats/EncodedTextReader.h \
  $$FBREADER_SRC/formats/util/TextFormatDetector.h \
  $$FBREADER_SRC/formats/util/MergedStream.h \
  $$FBREADER_SRC/formats/util/MiscUtil.h \
  $$FBREADER_SRC/formats/util/EntityFilesCollector.h \
  $$FBREADER_SRC/formats/util/XMLTextStream.h \
  $$FBREADER_SRC/formats/css/StyleSheetParser.h \
  $$FBREADER_SRC/formats/css/StyleSheetTable.h \
  $$FBREADER_SRC/formats/tcr/TcrStream.h \
  $$FBREADER_SRC/formats/tcr/TcrPlugin.h \
  $$FBREADER_SRC/formats/tcr/PPLBookReader.h \
  $$FBREADER_SRC/formats/rtf/RtfPlugin.h \
  $$FBREADER_SRC/formats/rtf/RtfBookReader.h \
  $$FBREADER_SRC/formats/rtf/RtfReader.h \
  $$FBREADER_SRC/formats/rtf/RtfReaderStream.h \
  $$FBREADER_SRC/formats/rtf/RtfImage.h \
  $$FBREADER_SRC/formats/rtf/RtfDescriptionReader.h \
  $$FBREADER_SRC/formats/FormatPlugin.h \
  $$FBREADER_SRC/formats/pdb/ZTXTStream.h \
  $$FBREADER_SRC/formats/pdb/PalmDocStream.h \
  $$FBREADER_SRC/formats/pdb/PdbReader.h \
  $$FBREADER_SRC/formats/pdb/PmlBookReader.h \
  $$FBREADER_SRC/formats/pdb/PdbStream.h \
  $$FBREADER_SRC/formats/pdb/PluckerImages.h \
  $$FBREADER_SRC/formats/pdb/PalmDocLikeStream.h \
  $$FBREADER_SRC/formats/pdb/HtmlMetainfoReader.h \
  $$FBREADER_SRC/formats/pdb/MobipocketHtmlBookReader.h \
  $$FBREADER_SRC/formats/pdb/BitReader.h \
  $$FBREADER_SRC/formats/pdb/HuffDecompressor.h \
  $$FBREADER_SRC/formats/pdb/DocDecompressor.h \
  $$FBREADER_SRC/formats/pdb/PmlReader.h \
  $$FBREADER_SRC/formats/pdb/PluckerBookReader.h \
  $$FBREADER_SRC/formats/pdb/PdbPlugin.h \
  $$FBREADER_SRC/formats/pdb/EReaderStream.h \
  $$FBREADER_SRC/formats/pdb/PluckerTextStream.h \
  $$FBREADER_SRC/formats/xhtml/XHTMLReader.h \
  $$FBREADER_SRC/formats/oeb/NCXReader.h \
  $$FBREADER_SRC/formats/oeb/OCFContainerReader.h \
  $$FBREADER_SRC/formats/oeb/OEBBookReader.h \
  $$FBREADER_SRC/formats/oeb/OEBTextStream.h \
  $$FBREADER_SRC/formats/oeb/OEBPlugin.h \
  $$FBREADER_SRC/formats/oeb/OEBMetaInfoReader.h \
  $$FBREADER_SRC/formats/oeb/OEBCoverReader.h \
  $$FBREADER_SRC/formats/html/HtmlPlugin.h \
  $$FBREADER_SRC/formats/html/HtmlEntityCollection.h \
  $$FBREADER_SRC/formats/html/HtmlReaderStream.h \
  $$FBREADER_SRC/formats/html/HtmlReader.h \
  $$FBREADER_SRC/formats/html/HtmlTagActions.h \
  $$FBREADER_SRC/formats/html/HtmlBookReader.h \
  $$FBREADER_SRC/formats/html/HtmlDescriptionReader.h \
  $$FBREADER_SRC/formats/chm/CHMPlugin.h \
  $$FBREADER_SRC/formats/chm/CHMFile.h \
  $$FBREADER_SRC/formats/chm/LZXDecompressor.h \
  $$FBREADER_SRC/formats/chm/CHMFileImage.h \
  $$FBREADER_SRC/formats/chm/HHCReferenceCollector.h \
  $$FBREADER_SRC/formats/chm/HtmlSectionReader.h \
  $$FBREADER_SRC/formats/chm/CHMReferenceCollection.h \
  $$FBREADER_SRC/formats/chm/HuffmanDecoder.h \
  $$FBREADER_SRC/formats/chm/BitStream.h \
  $$FBREADER_SRC/formats/chm/HHCReader.h \
  $$FBREADER_SRC/formats/txt/TxtBookReader.h \
  $$FBREADER_SRC/formats/txt/PlainTextFormat.h \
  $$FBREADER_SRC/formats/txt/TxtPlugin.h \
  $$FBREADER_SRC/formats/txt/TxtReader.h \
  $$FBREADER_SRC/bookmodel/FBTextKind.h \
  $$FBREADER_SRC/bookmodel/BookModel.h \
  $$FBREADER_SRC/bookmodel/BookReader.h
