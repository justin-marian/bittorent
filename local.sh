#!/bin/bash

cd "$(dirname "$0")" || exit 1

#=============================================================================
#================================= CONSTANTS =================================
#=============================================================================

RED='\033[0;31m'
NC='\033[0m'

DEFAULT_IMAGE_NAME="$(basename "$(pwd)")"
DEFAULT_TAG='latest'
DEFAULT_REGISTRY='gitlab.cs.pub.ro:5050'

MOUNT_PROJECT_DIRECTORY="/build/$USER/$(basename "$(pwd)")"

#=============================================================================
#=================================== UTILS ===================================
#=============================================================================

LOG_INFO() {
    echo -e "[$(date +%FT%T)] [INFO] $1"
}

LOG_FATAL() {
    echo -e "[$(date +%FT%T)] [${RED}FATAL${NC}] $1"
    exit 1
}

#=============================================================================
#=============================================================================
#=============================================================================

print_help() {
    echo "Usage:"
    echo ""
    echo "local.sh checker|docker|-h|--help"
    echo ""
    echo "      docker - runs docker commands to build, test or push an image made of this repo"
    echo "      checker - runs the checker"
    echo "      -h|--help - prints this message"
    echo ""
    echo "local.sh docker build [--image_name <image_name>] [--tag <tag>] [--registry <registry>] [--force_build]"
    echo ""
    echo "      --image_name <image_name> - the name of the image (default: current directory name)"
    echo "      --tag <tag> - the tag of the image (default: 'latest')"
    echo "      --registry <registry> - the registry in which the image will be pushed (default: 'gitlab.cs.pub.ro:5050')"
    echo "      --force_build - disable caching when building the docker image"
    echo ""
    echo "local.sh docker push --user <user> --token <token> [--image_name <image_name>] [--tag <tag>] [--registry <registry>]"
    echo ""
    echo "      --image_name <image_name> - the name of the image (default: current directory name)"
    echo "      --tag <tag> - the tag of the image (default: 'latest')"
    echo "      --registry <registry> - the registry in which the image will be pushed (default: 'gitlab.cs.pub.ro:5050')"
    echo "      --user <registry> - username for the repository registry"
    echo "      --token <token> - the token used to authenticate in the docker registry"
    echo ""
    echo "local.sh docker test [--full_image_name <full_image_name>] [argumets_for_checker]"
    echo ""
    echo "      --full_image_name <full_image_name> - the full name of the image (default: gitlab.cs.pub.ro:5050/<current_directory_name>:latest)"
    echo "      argumets_for_checker - list of space separated arguments to be passed to the checker"
    echo ""
    echo "local.sh docker interactive [--full_image_name <full_image_name>] [--use_executable <executbale>]"
    echo ""
    echo "      --full_image_name <full_image_name> - the full name of the image (default: gitlab.cs.pub.ro:5050/<current_directory_name>:latest)"
    echo "      --use_executable <executable> - command to run inside the container (default: /bin/bash)"
    echo ""
    echo "local.sh checker [--remove_image] [--use_existing_image <image_name>] [--force_build] [argumets_for_checker]"
    echo ""
    echo "      --remove_image - remove the checker's docker image after the run"
    echo "      --use_existing_image - user image_name instead of building the image from current directory"
    echo "      --force_build - disable caching when building the docker image"
    echo "      argumets_for_checker - list of space separated arguments to be passed to the checker"
    echo ""
}

docker_build() {
    local image_name="$DEFAULT_IMAGE_NAME"
    local tag="$DEFAULT_TAG"
    local registry="$DEFAULT_REGISTRY"
    local extra_docker_args=()

    while [[ $# -gt 0 ]]; do
        case $1 in
            --image_name)
                shift
                image_name="$1"
            ;;
            --tag)
                shift
                tag="$1"
            ;;
            --registry)
                shift
                registry="$1"
            ;;
            --force_build)
                extra_docker_args+=('--no-cache')
            ;;
        esac
        shift
    done

    LOG_INFO "Building Docker image..."

    docker image build "${extra_docker_args[@]}" -t "${registry}/${image_name}:${tag}" .
}

docker_push() {
    local image_name="$DEFAULT_IMAGE_NAME"
    local tag="$DEFAULT_TAG"
    local registry="$DEFAULT_REGISTRY"
    local user=''
    local token=''

    while [[ $# -gt 0 ]]; do
        case $1 in
            --image_name)
                shift
                image_name="$1"
            ;;
            --tag)
                shift
                tag="$1"
            ;;
            --registry)
                shift
                registry="$1"
            ;;
            --user)
                shift
                user="$1"
            ;;
            --token)
                shift
                token="$1"
            ;;
        esac
        shift
    done

    [ -z "$user" ] && LOG_FATAL "No user provided. Push operation will be aborted..."
    [ -z "$token" ] && LOG_FATAL "No token provided. Push operation will be aborted..."

    LOG_INFO "Pushing Docker image..."

    docker login "${registry}" -u "${user}" -p "${token}"
    docker push "${registry}/${image_name}:${tag}"
}

docker_test() {
    local full_image_name="${DEFAULT_REGISTRY}/${DEFAULT_IMAGE_NAME}:${DEFAULT_TAG}"

    while [[ $# -gt 0 ]]; do
        case $1 in
            --full_image_name)
                shift
                full_image_name="$1"
            ;;
        esac
        shift
    done

    checker_main --use_existing_image "$full_image_name" "$@"
}

docker_interactive() {
    local full_image_name="${DEFAULT_REGISTRY}/${DEFAULT_IMAGE_NAME}:${DEFAULT_TAG}"
    local executable="/bin/bash"

    while [[ $# -gt 0 ]]; do
        case $1 in
            --full_image_name)
                shift
                full_image_name="$1"
            ;;
            --use_executable)
                shift
                executable="$1"
            ;;
        esac
        shift
    done

    tmpdir="$(mktemp -d)"
    cp -R ./* "$tmpdir"

    docker run --rm -it \
            --mount type=bind,source="$tmpdir",target="$MOUNT_PROJECT_DIRECTORY" \
            --workdir "$MOUNT_PROJECT_DIRECTORY" \
            "$full_image_name" "$executable"
}

docker_main() {
    if [ "$1" = "build" ] ; then
        shift
        docker_build "$@"
    elif [ "$1" = "push" ] ; then
        shift
        docker_push "$@"
    elif [ "$1" = "test" ] ; then
        shift
        docker_test "$@"
    elif [ "$1" = "interactive" ] ; then
        shift
        docker_interactive "$@"
    fi
}

checker_main() {
    local script_args=()
    local remove_image=''
    local image_name=''
    local extra_docker_args=()

    while [[ $# -gt 0 ]]; do
        case $1 in
            --remove_image)
                remove_image='true'
            ;;
            --use_existing_image)
                shift
                image_name="$1"
            ;;
            --force_build)
                extra_docker_args+=('--no-cache')
            ;;
            *)
                script_args+=("$1")
            ;;
        esac
        shift
    done

    if [ -z "$image_name" ] ; then
        image_name="$(basename "$(pwd)")"

        LOG_INFO "Building image..."
        docker build "${extra_docker_args[@]}" -q -t "$image_name" .
    fi

    tmpdir="$(mktemp -d)"
    cp -R ./* "$tmpdir"

    LOG_INFO "Running checker..."

    # In your checker script if you must use absolute paths please use $CI_PROJECT_DIR to reference the location of your directory,
    # otherwise stick to relative paths.
    # It is guaranteed that the current working directory in which checker.sh will run is  $CI_PROJECT_DIR/checker.
    docker run --rm \
            --mount type=bind,source="$tmpdir",target="$MOUNT_PROJECT_DIRECTORY" \
            "$image_name" /bin/bash -c "rm -rf /usr/local/bin/bash; cd \"$MOUNT_PROJECT_DIRECTORY/build\"; \"$MOUNT_PROJECT_DIRECTORY/build/checker.sh\" \"${script_args[@]}\"" # remove bash middleware script

    if [ -n "$remove_image" ] ; then
        LOG_INFO "Cleaning up..."
        docker rmi -f "$image_name":latest
    fi

}

if [ "$1" = "checker" ] ; then
    shift
    checker_main "$@"
elif [ "$1" = "docker" ] ; then
    shift
    docker_main "$@"
elif [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    print_help
else
    print_help
fi
