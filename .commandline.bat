echo "# DIRECTX_PROJECT" >> README.md
git init
git add README.md
git add --all
git status
git commit -m "base commit"
git branch -M template_branch_1
git remote add origin https://github.com/Vasika-uso/DIRECTX_PROJECT.git
git push -u origin template_branch_1