if [ ! -z "$(git status --untracked-files=no  --porcelain)" ]; then
  echo "Script must be applied on a clean git state"
  exit 1
fi

clang-format-10 --version
echo

find . \( \( -name \*.c -o -name \*.h \) -a ! -iname \*soap\* \) -print0 | xargs -0 -n 1 clang-format -i --verbose

modifiedFiles=`git status --porcelain | grep '^ M' | cut -c4-`

if [[ -z $modifiedFiles ]]; then
  echo "Formatting passed!"
  exit 0;
else
  echo "The following files have clang-format problems:"
  git diff --stat $modifiedFiles
  echo "Please run code check before committing: "
  echo
  echo "find . \( \( -name \*.c -o -name \*.h \) -a ! -iname \*soap\* \) -print0 | xargs -0 -n 1 clang-format --Werror -i --verbose"
  git reset HEAD --hard
fi

exit 1