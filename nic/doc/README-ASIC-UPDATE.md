README for updating capri asic pointer and asic files
=====================================================
Note: All of the following steps needs to be performed on srv13 server as jenkins user

    1) Prepare the headers and sources
        a) Add the required header files to nic/tools/asic_sw_combined_sanity/asic_headers.txt
        b) Add the required source files to nic/sdk/platform/capri/csrint/module.mk.asic_sw_combined_sanity

    2) Generate and copy diffs
        a) Generate the diff for SW repo and copy the diff as jenkins user to:
           /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/sw/
        b) Update the version of third_party_libs in /sw/minio/VERSIONS in SW repo and copy the diff as jenkins user to:
           /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/sw/
           (This step is required since we need to increment the version number whenever third party libs including updated
           ASIC lib are pushed to minio. See #4 below.)
        c) If corresponding ASIC changes are required, then copy the asic diff to
           /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/asic

    3) Trigger the asic-sw-combined-sanity using the below link. Specify the required ASIC_BRANCH and ASSET_LABEL (ASIC SHA) in the link.
       http://srv13:8080/view/CombinedSanity/job/ASIC_SW_COMBINED_SANITY/

    4) The ASIC_SW_COMBINED_SANITY uses /var/lib/jenkins/scripts/asic_sw_combined_sanity.sh to:
        a) Pull SW repo
        b) Pull ASIC repo based on the specified SHA
        c) Apply SW patches from /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/sw/
        d) Apply ASIC patches from /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/asic/
        d) Copy the asic header files specified in nic/tools/asic_sw_combined_sanity/asic_headers.txt to
           nic/sdk/third-party/asic/
        e) Build the x86_64 and aarch64 asic-libs for the sources specified in
           nic/sdk/platform/capri/csrint/module.mk.asic_sw_combined_sanity
        f) copy the asic-libs to nic/sdk/third-party/asic/
        g) Build x86_64 and aarch64 NIC targets
        h) Push assets based on the ASIC SHA
        i) Increment third-party version and push it to the assets
        j) Push the changes to branch - vikasbd:asic_label_update_<ASIC_BRANCH>

    6) If any change in /var/lib/jenkins/scripts/asic_sw_combined_sanity.sh, also update in SW repo
       nic/tools/asic_sw_combined_sanity/asic_sw_combined_sanity.sh

    7) Use the below link to raise PR for the changes:
       https://github.com/pensando/sw/compare/master...vikasbd:asic_label_update_<ASIC_BRANCH>

    At the start of jenkins job you can find the build machine and workspace information. You can login to this build
    machine and go to the workspace to check if anything something is missing after your build
