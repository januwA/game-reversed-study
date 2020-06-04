const fs = require("fs");
const path = require("path");

const exclude_file = [/^\./, /(node_modules|images|docs|readme\.md)/i];

function scan(dirPath, parent) {
  const files = fs
    .readdirSync(dirPath)
    .filter((name) => !exclude_file.some((e) => e.test(name)));

  for (const fp of files) {
    let p = path.posix.join(dirPath, fp);
    const stats = fs.statSync(p);
    if (stats.isDirectory()) {
      const newParent = {
        title: p.split("/").slice(-1)[0],
        children: [],
      };
      parent.push(newParent);
      scan(p, newParent.children);
    } else {
      const ext = path.extname(fp);
      if (ext !== ".md") continue;
      p = "/" + p;
      const _name = path.basename(p).split(".")[0];
      if (dirPath === "./") {
        parent[0].children.push([p, _name]);
      } else {
        parent.push([p, _name]);
      }
    }
  }
}

const sidebar = [];
sidebar.push({
  title: "/",
  children: [],
});
scan("./", sidebar);

const config = {
  title: "记录游戏逆向学习",
  description: "记录游戏逆向学习",
  dest: "./docs",
  base: "/game-reversed-study/",
  themeConfig: {
    search: true,
    repo: "januwA/game-reversed-study",
    sidebar: sidebar,
  },
};

fs.writeFileSync(
  "./.vuepress/config.js",
  `module.exports = ${JSON.stringify(config, null, "  ")}`
);
