Invoke-Expression "git submodule update --init"

# the sound banks (*.ifbsnd) are stored with git lfs
# this sets up lfs for this repo, and downloads the banks if the clone was made without it
if (-not (Get-Command git-lfs -ErrorAction SilentlyContinue)) {
    throw "git lfs was not found, install it from https://git-lfs.com and run this again"
}
Invoke-Expression "git lfs install --local"
Invoke-Expression "git lfs pull"
