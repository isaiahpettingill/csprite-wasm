import { copyFileSync, mkdirSync, rmSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";

const packageRoot = dirname(dirname(fileURLToPath(import.meta.url)));
const repoRoot = join(packageRoot, "..", "..");

const build = spawnSync("sh", [join(repoRoot, "build_web.sh")], {
  cwd: packageRoot,
  stdio: "inherit",
});

if (build.status !== 0) {
  process.exit(build.status ?? 1);
}

const dist = join(packageRoot, "dist");
rmSync(dist, { force: true, recursive: true });
mkdirSync(dist, { recursive: true });
copyFileSync(join(repoRoot, "build", "web", "csprite_api.js"), join(dist, "csprite_api.js"));
copyFileSync(join(repoRoot, "build", "web", "csprite_api.wasm"), join(dist, "csprite_api.wasm"));
