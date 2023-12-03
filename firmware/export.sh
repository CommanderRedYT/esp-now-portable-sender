PROJECT_ROOT="$(dirname "$BASH_SOURCE")"

if [[ ! -f "${PROJECT_ROOT}/esp-idf/export.sh" ]]
then
    echo "esp-idf is missing, please check out all needed submodules!"
    echo "git submodule update --init --recursive"
    return
fi

complete -F _switchconf_completions ./switchconf.sh

function _switchconf_completions()
{
  if [[ $COMP_CWORD != 1 ]]
  then
    return 1
  fi

  COMPREPLY=($(compgen -W "$(./switchconf.sh --list)" "${COMP_WORDS[1]}"))
}

export PATH=$PATH:$(pwd)/tools

. ${PROJECT_ROOT}/esp-idf/export.sh
