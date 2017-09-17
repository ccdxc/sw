from "srv1.pensando.io:5000/pensando/nic:dependencies"

BASE_BUILD_DIR = "/tmp/build"
run "yum install -y dkms iproute2 net-tools zip zlib1g-dev"
inside BASE_BUILD_DIR do
  run "wget https://github.com/bazelbuild/bazel/releases/download/0.5.4/bazel-0.5.4-installer-linux-x86_64.sh"
  run "chmod +x bazel-0.5.4-installer-linux-x86_64.sh"
  run "./bazel-0.5.4-installer-linux-x86_64.sh"
end

inside "/etc" do
    run "rm localtime"
    run "ln -s /usr/share/zoneinfo/US/Pacific localtime"
end 

# in the CI this block is triggered; in the Makefiles it is not.
if getenv("NO_COPY") == ""
  copy ".", ".", ignore_list: %w[.git]
end

workdir "/sw/nic"

tag "pensando/nic"
