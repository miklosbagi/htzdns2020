### HTZ Params Parser

function PARAMS_PARSER {
while [ "$1" != "" ]; do
  case $1 in
    -r | r | --rebuild )			 rebuild="1";;
    -t | t | --test )				 runtests="1";;
    -h | h | help | -help | /help| --help )      USAGE; exit;;
    * )                                          if [ -f $1 ]; then file="$1"; else usage; exit 1; fi;
  esac
  shift
done
}

### Usage manual next to params, so we can keep it updated.
function USAGE {
  echo " ℹ️  Usage params: [ -h | h | --help ]"
  echo "                  [ -r | r | --rebuild ]"
  echo
  echo "   -r | r | --rebuild	Run configuration builder."
  echo "   -t | t | --test      Run functional tests."
  echo "   -h | h | --help	This usage manual."
  echo ""
}
