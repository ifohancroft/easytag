/*
 * - bar.h : defines constant associated to the action "<menuitem action="
 * - bar.c : Create_UI() does the actions
 */

static const gchar *ui_xml =
"<ui>"

/*
 * Menu bar
 */
"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"

"      <menuitem action='OpenFile' />"
"      <menuitem action='RunAudio' />"
"      <separator />"

"      <menuitem action='SelInv' />"
"      <separator />"

"      <menuitem action='DeleteFile' />"
"      <separator />"

"      <menuitem action='UndoFile' />"
"      <menuitem action='RedoFile' />"
"      <menuitem action='SaveFile' />"
"      <menuitem action='SaveFileForced' />"
"      <separator />"

"      <menuitem action='Quit' />"
"    </menu>"

"    <menu action='ViewMenu'>"
"      <menuitem action='ShowScanner' />"
"      <menu action='ScannerMenu'>"
"        <menuitem action='FillTag' />"
"        <menuitem action='RenameFile' />"
"        <menuitem action='ProcessFields' />"
"      </menu>"
"      <separator />"

"      <menuitem action='TreeViewMode'/>"
"      <menuitem action='ArtistViewMode'/>"
"      <separator />"

"      <menu action='SortTagMenu'>"
"        <menuitem action='SortTrackNumAsc' />"
"        <menuitem action='SortTrackNumDesc' />"
"        <separator />"
"        <menuitem action='SortTitleAsc' />"
"        <menuitem action='SortTitleDesc' />"
"        <separator />"
"        <menuitem action='SortArtistAsc' />"
"        <menuitem action='SortArtistDesc' />"
"        <separator />"
"        <menuitem action='SortAlbumArtistAsc' />"
"        <menuitem action='SortAlbumArtistDesc' />"
"        <separator />"
"        <menuitem action='SortAlbumAsc' />"
"        <menuitem action='SortAlbumDesc' />"
"        <separator />"
"        <menuitem action='SortYearAsc' />"
"        <menuitem action='SortYearDesc' />"
"        <separator />"
"        <menuitem action='SortYearAsc' />"
"        <menuitem action='SortYearDesc' />"
"        <separator />"
"        <menuitem action='SortGenreAsc' />"
"        <menuitem action='SortGenreDesc' />"
"        <separator />"
"        <menuitem action='SortCommentAsc' />"
"        <menuitem action='SortCommentDesc' />"
"        <separator />"
"        <menuitem action='SortComposerAsc' />"
"        <menuitem action='SortComposerDesc' />"
"        <separator />"
"        <menuitem action='SortOrigArtistAsc' />"
"        <menuitem action='SortOrigArtistDesc' />"
"        <separator />"
"        <menuitem action='SortCopyrightAsc' />"
"        <menuitem action='SortCopyrightDesc' />"
"        <separator />"
"        <menuitem action='SortUrlAsc' />"
"        <menuitem action='SortUrlDesc' />"
"        <separator />"
"        <menuitem action='SortEncodedByAsc' />"
"        <menuitem action='SortEncodedByDesc' />"
"      </menu>"

"      <menu action='SortPropMenu'>"
"        <menuitem action='SortFilenameAsc' />"
"        <menuitem action='SortFilenameDesc' />"
"        <separator />"
"        <menuitem action='SortDateAsc' />"
"        <menuitem action='SortDateDesc' />"
"        <separator />"
"        <menuitem action='SortTypeAsc' />"
"        <menuitem action='SortTypeDesc' />"
"        <separator />"
"        <menuitem action='SortSizeAsc' />"
"        <menuitem action='SortSizeDesc' />"
"        <separator />"
"        <menuitem action='SortDurationAsc' />"
"        <menuitem action='SortDurationDesc' />"
"        <separator />"
"        <menuitem action='SortBitrateAsc' />"
"        <menuitem action='SortBitrateDesc' />"
"        <separator />"
"        <menuitem action='SortSamplerateAsc' />"
"        <menuitem action='SortSamplerateDesc' />"
"      </menu>"
"      <separator />"

"      <menuitem action='CollapseTree' />"
"      <menuitem action='RefreshTree' />"
"      <separator />"

"      <menuitem action='ReloadDir' />"
"      <separator />"

#ifndef G_OS_WIN32
"      <menuitem action='BrowseHiddenDir' />"
#endif /* !G_OS_WIN32 */
"    </menu>"

"  </menubar>"


/*
 * Tool bar
 */
"  <toolbar name='ToolBar'>"
"    <toolitem action='ShowScanner'/>"
"    <toolitem action='UndoFile'/>"
"    <toolitem action='RedoFile'/>"
"    <toolitem action='SaveFile'/>"
"    <separator />"

"    <toolitem action='TreeViewMode'/>"
"    <toolitem action='ArtistViewMode'/>"
"    <separator />"

"    <toolitem action='SelInv'/>"
"    <separator />"

"    <toolitem action='Stop'/>"
"  </toolbar>"


/*
 * Popup menus
 */
// Popup in file list
"  <popup name='FilePopup'>"
"    <menuitem action='SelAll' />"
"    <menuitem action='UnselAll' />"
"    <menuitem action='SelInv' />"
"    <separator />"
"    <menuitem action='RunAudio' />"
"    <separator />"
"    <menu action='ScannerSubpopup'>"
"      <menuitem action='ShowScanner' />"
"    </menu>"
"    <menuitem action='CDDBSearchFile' />"
"    <menuitem action='SearchFile' />"
"    <menuitem action='DeleteFile' />"
"    <menuitem action='ReloadDir' />"
"    <menuitem action='OpenFile' />"
"    <separator />"

"    <menu action='SortTagMenu'>"
"      <menuitem action='SortTrackNumAsc' />"
"      <menuitem action='SortTrackNumDesc' />"
"      <separator />"
"      <menuitem action='SortTitleAsc' />"
"      <menuitem action='SortTitleDesc' />"
"      <separator />"
"      <menuitem action='SortArtistAsc' />"
"      <menuitem action='SortArtistDesc' />"
"      <separator />"
"      <menuitem action='SortAlbumArtistAsc' />"
"      <menuitem action='SortAlbumArtistDesc' />"
"      <separator />"
"      <menuitem action='SortAlbumAsc' />"
"      <menuitem action='SortAlbumDesc' />"
"      <separator />"
"      <menuitem action='SortYearAsc' />"
"      <menuitem action='SortYearDesc' />"
"      <separator />"
"      <menuitem action='SortYearAsc' />"
"      <menuitem action='SortYearDesc' />"
"      <separator />"
"      <menuitem action='SortGenreAsc' />"
"      <menuitem action='SortGenreDesc' />"
"      <separator />"
"      <menuitem action='SortCommentAsc' />"
"      <menuitem action='SortCommentDesc' />"
"      <separator />"
"      <menuitem action='SortComposerAsc' />"
"      <menuitem action='SortComposerDesc' />"
"      <separator />"
"      <menuitem action='SortOrigArtistAsc' />"
"      <menuitem action='SortOrigArtistDesc' />"
"      <separator />"
"      <menuitem action='SortCopyrightAsc' />"
"      <menuitem action='SortCopyrightDesc' />"
"      <separator />"
"      <menuitem action='SortUrlAsc' />"
"      <menuitem action='SortUrlDesc' />"
"      <separator />"
"      <menuitem action='SortEncodedByAsc' />"
"      <menuitem action='SortEncodedByDesc' />"
"      <separator />"
"    </menu>"

"    <menu action='SortPropMenu'>"
"      <menuitem action='SortFilenameAsc' />"
"      <menuitem action='SortFilenameDesc' />"
"      <separator />"
"      <menuitem action='SortDateAsc' />"
"      <menuitem action='SortDateDesc' />"
"      <separator />"
"      <menuitem action='SortTypeAsc' />"
"      <menuitem action='SortTypeDesc' />"
"      <separator />"
"      <menuitem action='SortSizeAsc' />"
"      <menuitem action='SortSizeDesc' />"
"      <separator />"
"      <menuitem action='SortDurationAsc' />"
"      <menuitem action='SortDurationDesc' />"
"      <separator />"
"      <menuitem action='SortBitrateAsc' />"
"      <menuitem action='SortBitrateDesc' />"
"      <separator />"
"      <menuitem action='SortSamplerateAsc' />"
"      <menuitem action='SortSamplerateDesc' />"
"    </menu>"
"  </popup>"

// Popup in browser tree
"  <popup name='DirPopup'>"
"    <menuitem action='DirPopupRunAudio' />"
"    <separator />"
"    <menuitem action='GoToHome' />"
"    <menuitem action='GoToDefaultPath' />"
"    <menuitem action='SetDefaultPath' />"
"    <separator />"
"    <menuitem action='RenameDir' />"
"    <menuitem action='ReloadDir' />"
"    <menuitem action='BrowseDir' />"
"    <separator />"
"    <menuitem action='BrowseSubdir' />"
#ifndef G_OS_WIN32
"    <menuitem action='BrowseHiddenDir' />"
#endif /* !G_OS_WIN32 */
"    <separator />"
"    <menuitem action='CollapseTree' />"
"    <menuitem action='RefreshTree' />"
"  </popup>"

// Popup in browser artist list
"  <popup name='DirArtistPopup'>"
"    <menuitem action='ArtistRunAudio' />"
//"    <separator />"
//"    <menuitem action='ArtistOpenFile' />"
"  </popup>"

// Popup in browser album list
"  <popup name='DirAlbumPopup'>"
"    <menuitem action='AlbumRunAudio' />"
//"    <separator />"
//"    <menuitem action='AlbumOpenFile' />"
"  </popup>"

// Popup in Log list
"  <popup name='LogPopup'>"
"    <menuitem action='CleanLog' />"
"  </popup>"

"</ui>";
