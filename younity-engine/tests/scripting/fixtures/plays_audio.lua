return {
    start = function(entity)
        audio.play("sounds/jump.wav", 0.5)
        audio.play("sounds/dash.wav")
        audio.playMusic("music/theme.ogg")
        audio.playMusic("music/end.ogg", false)
        audio.pauseMusic()
        audio.stopMusic()
        audio.setVolume(0.25)
        audio.setMusicVolume(1)
    end,
}
