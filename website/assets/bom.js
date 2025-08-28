(function(){
  async function renderBOM(mountId, csvPath){
    const mount = document.getElementById(mountId);
    if (!mount) return;
    try {
      const res = await fetch(csvPath + '?_=' + Date.now());
      const csv = await res.text();
      const rows = csv.trim().split(/\r?\n/).map(r => {
        // Simple CSV parsing that handles commas in quoted fields
        const result = [];
        let current = '';
        let inQuotes = false;
        
        for (let i = 0; i < r.length; i++) {
          const char = r[i];
          if (char === '"') {
            inQuotes = !inQuotes;
          } else if (char === ',' && !inQuotes) {
            result.push(current.trim());
            current = '';
          } else {
            current += char;
          }
        }
        result.push(current.trim());
        return result;
      });
      const [header, ...raw] = rows;

      const keys = [
        { key: 'Category', label: 'Category' },
        { key: 'Item', label: 'Item' },
        { key: 'MPN', label: 'MPN' },
        { key: 'Supplier', label: 'Supplier' },
        { key: 'Qty', label: 'Qty' },
        { key: 'Unit', label: 'Unit $ (1k)' },
        { key: 'Notes', label: 'Notes' },
        { key: 'Package', label: 'Package' },
      ];

      const toNum = (v) => { const n = parseFloat(v); return isFinite(n) ? n : 0; };
      const data = raw.map(r => ({
        Category: r[0],
        Item: r[1],
        MPN: r[2],
        Supplier: r[3],
        Qty: toNum(r[4]),
        Unit: toNum(r[5]),
        Notes: r[6] || '',
        Package: r[7] || ''
      })).map(d => ({ ...d, Ext: d.Qty * d.Unit }));

      // Controls
      const controls = document.createElement('div');
      controls.className = 'bom-controls';
      const search = document.createElement('input');
      search.type = 'search';
      search.placeholder = 'Filter… (Category, Item, MPN, Supplier, Notes, Package)';
      const cat = document.createElement('select');
      const categories = Array.from(new Set(data.map(d => d.Category))).sort();
      cat.innerHTML = '<option value="">All categories</option>' + categories.map(c => `<option>${c}</option>`).join('');
      controls.appendChild(search);
      controls.appendChild(cat);

      // Table
      const table = document.createElement('table');
      table.className = 'bom-table';
      const thead = document.createElement('thead');
      const headRow = document.createElement('tr');
      keys.forEach(k => {
        const th = document.createElement('th');
        th.textContent = k.label;
        th.dataset.key = k.key;
        th.style.cursor = 'pointer';
        headRow.appendChild(th);
      });
      const thExt = document.createElement('th');
      thExt.textContent = 'Ext $ (1k)';
      thExt.dataset.key = 'Ext';
      headRow.appendChild(thExt);
      thead.appendChild(headRow);
      table.appendChild(thead);
      const tbody = document.createElement('tbody');
      table.appendChild(tbody);
      const tfoot = document.createElement('tfoot');
      table.appendChild(tfoot);

      // Layout wrapper for table + summary
      const wrap = document.createElement('div');
      wrap.className = 'bom-wrap';
      const summary = document.createElement('div');
      summary.className = 'bom-summary';

      mount.innerHTML = '';
      mount.appendChild(controls);
      wrap.appendChild(table);
      wrap.appendChild(summary);
      mount.appendChild(wrap);

      let sortKey = 'Category';
      let sortDir = 1; // 1 asc, -1 desc

      function applyFilters(list){
        const q = (search.value || '').toLowerCase();
        const c = cat.value || '';
        return list.filter(d => (!c || d.Category === c) && (!q || (
          (d.Category||'').toLowerCase().includes(q) ||
          (d.Item||'').toLowerCase().includes(q) ||
          (d.MPN||'').toLowerCase().includes(q) ||
          (d.Supplier||'').toLowerCase().includes(q) ||
          (d.Notes||'').toLowerCase().includes(q) ||
          (d.Package||'').toLowerCase().includes(q)
        )));
      }

      function sortList(list){
        return list.slice().sort((a,b) => {
          const va = a[sortKey]; const vb = b[sortKey];
          const na = typeof va === 'number'; const nb = typeof vb === 'number';
          if (na || nb) return sortDir * (toNum(va) - toNum(vb));
          return sortDir * String(va).localeCompare(String(vb));
        });
      }

      function render(){
        const filtered = sortList(applyFilters(data));
        // body
        tbody.innerHTML = '';
        filtered.forEach(d => {
          const tr = document.createElement('tr');
          const cells = [d.Category, d.Item, d.MPN, d.Supplier, d.Qty, d.Unit.toFixed(2), d.Notes, '$'+(d.Ext).toFixed(2)];
          tr.innerHTML = cells.map(c => '<td>'+c+'</td>').join('');
          tbody.appendChild(tr);
        });
        // totals in separate summary panel
        const groupTotals = {};
        const total = filtered.reduce((s,d)=>{ const ext=d.Ext; groupTotals[d.Category]=(groupTotals[d.Category]||0)+ext; return s+ext; },0);
        tfoot.innerHTML = '';
        const list = Object.entries(groupTotals)
          .sort((a,b)=> b[1]-a[1])
          .map(([k,v]) => `<div class="row"><span>${k}</span><strong>$${v.toFixed(2)}</strong></div>`)
          .join('');
        summary.innerHTML = `
          <div class="total"><div>Total (filtered)</div><div class="value">$${total.toFixed(2)}</div></div>
          <details class="split"><summary>Category breakdown</summary>${list}</details>
        `;
        // Notify listeners so the page can aggregate totals
        try {
          window.dispatchEvent(new CustomEvent('EdgeBOM:totals', { detail: { mountId, csvPath, total, breakdown: groupTotals } }));
        } catch (_) {}
      }

      thead.addEventListener('click', (e) => {
        const th = e.target.closest('th');
        if (!th || !th.dataset.key) return;
        const key = th.dataset.key;
        if (sortKey === key) sortDir *= -1; else { sortKey = key; sortDir = 1; }
        render();
      });
      search.addEventListener('input', render);
      cat.addEventListener('change', render);

      render();
    } catch (err) {
      mount.textContent = 'Failed to load BOM.';
    }
  }

  window.EdgeBOM = { renderBOM };
})();
