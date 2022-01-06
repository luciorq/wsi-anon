if [ ! -z "$(git status --untracked-files=no  --porcelain)" ]; then
  echo "Script must be applied on a clean git state"
  exit 1
fi

echo "Used clang version:"
clang-format --version
echo

find . \( \( -name \*.c -o -name \*.h \) -a ! -iname \*soap\* \) -print0 | xargs -0 -n 1 clang-format-10 -i --verbose

modifiedFiles==`git status --porcelain | grep '^ M' | cut -c4-`

if [[ -z $modifiedFiles ]]; then
  echo "Formatting passed!"
  exit 0;
else
  echo "The following files have clang-format problems:"
  git diff --stat $modifiedFiles
  echo "Please run"
  echo
  echo "find . \( \( -name \*.c -o -name \*.h \) -a ! -iname \*soap\* \) -print0 | xargs -0 -n 1 clang-format-10 --Werror -i --verbose"
  echo
  echo "to solve the issue."
  git reset HEAD --hard
fi