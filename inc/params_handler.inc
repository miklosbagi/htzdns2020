### HTZ Params Parser

function PARAMS_PARSER {
while [ "$1" != "" ]; do
  case $1 in
    -r | r | --rebuild )			 rebuild="1";;
    -t | t | --test )				 runtests="1";;
    -tc | tc | --tests-coverage )                coverage="1";;
    -p | p | --pretend )			 pretend="1";;
    -v | v | --verbose )			 verbose="1";;
    -h | h | help | -help | /help| --help )      USAGE; exit;;
    * )                                          _WRN "Unknown argument provided (${@#})"; USAGE; exit 1 ;;
  esac
  shift
done
}

### Usage manual next to params, so we can keep it updated.
function USAGE {
  $echo
  $echo " 🌎 htzdns: A DNS record updater for dynamic IP addresses for domains managed at hetzner.de"
  $echo
  $echo " ℹ️  Usage params: [ -h | h | --help ]"
  $echo "                  [ -r | r | --rebuild ]"
  $echo "                  [ -p -v | p v | --pretend --verbose ]"
  $echo "                  [ -t | t | --test ]"
  $echo
  $echo "   -r | r | --rebuild	Run configuration builder (interactive)"
  $echo "   -p | p | --pretend   Do not make any changes, only show what would be done."
  $echo "   -v | v | --verbose   See what's happening."
  $echo "   -t | t | --test      Run tests (confirms compatibilty, warns on significant API changes)"
  $echo "   -h | h | --help	This usage manual."
  $echo
}
