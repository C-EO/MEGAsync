import common 1.0

// Error panel for backup items: same behaviour as SettingsError with the
// backup-specific texts, icons and remove action.
SettingsError {
    showRestore: false
    retryOnIgnoreFileError: false
    enableText: SettingsStrings.solveIssueEnableBackup
    startNewText: SettingsStrings.solveIssueStartNewBackup
    startNewIcon: Images.database_plus_small_thin_outline
    removeText: SettingsStrings.menuActionsStopBackup
    removeIcon: Images.database_x_medium_thin_outline
    removeAction: function() { settingsAccess.remove(itemIndex); }
    buttonVerticalPadding: 3
}
