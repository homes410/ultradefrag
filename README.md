Created from below command.
```
P=ultradefrag
SESSION=~/.ssh/key
rsync -av --no-p --no-o --no-g rsync://svn.code.sf.net/p/$P $P-svn
mkdir $P
cd $P
git svn init file://../$P-svn
git svn migrate
git svn fetch --all
git filter-branch --msg-filter 'sed -e "s/file.*\/'$P'-svn@/https:
\/\/svn.code.sf.net\/p\/'$P'\/code@/"' master
ssh-agent sh -c "ssh-add $SESSION;git push -u origin master"
'''
