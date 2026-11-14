// Wraps every <table> of rendered Markdown in a scroll container, so a wide table scrolls
// inside its own box on narrow screens instead of widening the whole article
// (`.doc-content .table-scroll` in public/theme.css).
export function wrapTables(html) {
  return html
    .replace(/<table(\s[^>]*)?>/g, tag => `<div class="table-scroll">${tag}`)
    .replace(/<\/table>/g, '</table></div>')
}
