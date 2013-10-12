if not exist Output mkdir Output
if not exist Output\html mkdir Output\html
if not exist Output\icons mkdir Output\icons
if not exist Output\scripts mkdir Output\scripts
if not exist Output\styles mkdir Output\styles
if not exist Output\media mkdir Output\media
copy "%BSHLP%\Presentation\safmq.net\icons\*" Output\icons
copy "%BSHLP%\Presentation\safmq.net\scripts\*" Output\scripts
copy "%BSHLP%\Presentation\safmq.net\styles\*" Output\styles
if not exist Intellisense mkdir Intellisense

