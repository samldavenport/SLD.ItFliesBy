local overseer = require("overseer")

overseer.register_template({
    name = "Build Debug",

    builder = function()
        return {
            cmd = {
                "pwsh.exe"
            },
            args = {
                "-NoProfile",
                "-ExecutionPolicy", "Bypass",
                "-File", vim.fn.getcwd() .. "/scripts/Invoke-BuildDebug.ps1",
            },
            components = {
                "default",
            },
        }
    end,
})
vim.keymap.set("n", "<C-b>", function()
    require("overseer").run_template({
        name = "Build Debug",
    })
end, {})

overseer.register_template({
    name = "Run Debug",

    builder = function()
        return {
            cmd = {
                vim.fn.getcwd() .. "/build/debug/bin/ItFliesBy.exe",
            },

            cwd = vim.fn.getcwd() .. "/build/debug/bin",
            components = {
                "default",
            },
        }
    end,
})
vim.keymap.set("n", "<F5>", function()
    require("overseer").run_template({
        name = "Run Debug",
    })
end, {})

