const canvas = document.getElementById('main-canvas');
const ctx = canvas.getContext('2d');
const wrapper = document.getElementById('canvas-wrapper');
const coordDisplayX = document.getElementById('cur-x');
const coordDisplayY = document.getElementById('cur-y');
const pointsList = document.getElementById('points-list');
const pointCountDisplay = document.getElementById('point-count');

// Settings
const bindKeyInput = document.getElementById('bind-key');
const folderNameInput = document.getElementById('folder-name');
const defaultCmdsInput = document.getElementById('default-cmds');

// State
let points = [];
let zoom = 1;
let offset = { x: 0, y: 0 };
let isDragging = false;
let lastMousePos = { x: 0, y: 0 };
let selectedPointIndex = null;

const TILE_SIZE = 32;

function init() {
    resize();
    window.addEventListener('resize', resize);

    // Canvas events
    wrapper.addEventListener('mousedown', startDrag);
    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('mouseup', stopDrag);
    wrapper.addEventListener('wheel', handleWheel, { passive: false });
    wrapper.addEventListener('click', handleCanvasClick);

    // UI events
    document.getElementById('generate-btn').onclick = generateConfigs;
    document.getElementById('clear-all-btn').onclick = clearAll;
    document.getElementById('zoom-in').onclick = () => { zoom *= 1.2; render(); };
    document.getElementById('zoom-out').onclick = () => { zoom /= 1.2; render(); };
    document.getElementById('reset-view').onclick = () => { zoom = 1; offset = { x: 0, y: 0 }; render(); };

    document.getElementById('modal-save').onclick = savePointCmds;
    document.getElementById('modal-close').onclick = closeModal;
    document.getElementById('close-overlay').onclick = () => document.getElementById('success-overlay').classList.add('hidden');
    document.getElementById('copy-start-cmd').onclick = copyStartCmd;
    document.getElementById('download-zip').onclick = downloadZip;

    document.getElementById('export-state-btn').onclick = exportState;
    document.getElementById('import-state-btn').onclick = () => {
        const input = document.createElement('input');
        input.type = 'file';
        input.onchange = e => {
            const file = e.target.files[0];
            const reader = new FileReader();
            reader.onload = () => {
                const data = JSON.parse(reader.result);
                points = data.points || [];
                bindKeyInput.value = data.settings.bindKey;
                folderNameInput.value = data.settings.folderName;
                defaultCmdsInput.value = data.settings.defaultCmds;
                updatePointsList();
                render();
            };
            reader.readAsText(file);
        };
        input.click();
    };

    render();
}

function resize() {
    canvas.width = wrapper.clientWidth;
    canvas.height = wrapper.clientHeight;
    render();
}

function startDrag(e) {
    if (e.button === 1 || (e.button === 0 && e.altKey)) { // Middle click or Alt+Left
        isDragging = true;
        lastMousePos = { x: e.clientX, y: e.clientY };
    }
}

function stopDrag() {
    isDragging = false;
}

function handleMouseMove(e) {
    const rect = canvas.getBoundingClientRect();
    const x = (e.clientX - rect.left - canvas.width / 2 - offset.x) / (TILE_SIZE * zoom);
    const y = (e.clientY - rect.top - canvas.height / 2 - offset.y) / (TILE_SIZE * zoom);

    coordDisplayX.textContent = Math.floor(x);
    coordDisplayY.textContent = Math.floor(y);

    if (isDragging) {
        offset.x += e.clientX - lastMousePos.x;
        offset.y += e.clientY - lastMousePos.y;
        lastMousePos = { x: e.clientX, y: e.clientY };
        render();
    }
}

function handleWheel(e) {
    e.preventDefault();
    const zoomSpeed = 0.1;
    if (e.deltaY < 0) zoom *= (1 + zoomSpeed);
    else zoom /= (1 + zoomSpeed);
    render();
}

function handleCanvasClick(e) {
    if (isDragging || e.button !== 0 || e.altKey) return;

    const rect = canvas.getBoundingClientRect();
    const x = Math.floor((e.clientX - rect.left - canvas.width / 2 - offset.x) / (TILE_SIZE * zoom));
    const y = Math.floor((e.clientY - rect.top - canvas.height / 2 - offset.y) / (TILE_SIZE * zoom));

    // Check if clicked near an existing point to delete/edit
    const existingIndex = points.findIndex(p => p.x === x && p.y === y);
    if (existingIndex !== -1) {
        // Just select it for now
        return;
    }

    points.push({ x, y, cmds: "" });
    updatePointsList();
    render();
}

function render() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const centerX = canvas.width / 2 + offset.x;
    const centerY = canvas.height / 2 + offset.y;
    const currentTileSize = TILE_SIZE * zoom;

    // Draw Grid
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.05)';
    ctx.lineWidth = 1;

    const startX = Math.floor((-canvas.width / 2 - offset.x) / currentTileSize);
    const endX = Math.ceil((canvas.width / 2 - offset.x) / currentTileSize);
    const startY = Math.floor((-canvas.height / 2 - offset.y) / currentTileSize);
    const endY = Math.ceil((canvas.height / 2 - offset.y) / currentTileSize);

    for (let i = startX; i <= endX; i++) {
        ctx.beginPath();
        ctx.moveTo(centerX + i * currentTileSize, 0);
        ctx.lineTo(centerX + i * currentTileSize, canvas.height);
        ctx.stroke();
    }
    for (let j = startY; j <= endY; j++) {
        ctx.beginPath();
        ctx.moveTo(0, centerY + j * currentTileSize);
        ctx.lineTo(canvas.width, centerY + j * currentTileSize);
        ctx.stroke();
    }

    // Draw Axes
    ctx.strokeStyle = 'rgba(99, 102, 241, 0.3)';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(centerX, 0);
    ctx.lineTo(centerX, canvas.height);
    ctx.stroke();
    ctx.beginPath();
    ctx.moveTo(0, centerY);
    ctx.lineTo(canvas.width, centerY);
    ctx.stroke();

    // Draw Points
    points.forEach((p, index) => {
        const px = centerX + p.x * currentTileSize;
        const py = centerY + p.y * currentTileSize;

        // Point shadow
        ctx.fillStyle = 'rgba(99, 102, 241, 0.4)';
        ctx.beginPath();
        ctx.arc(px + currentTileSize / 2, py + currentTileSize / 2, 6 * zoom, 0, Math.PI * 2);
        ctx.fill();

        // Point
        ctx.fillStyle = '#6366f1';
        ctx.beginPath();
        ctx.arc(px + currentTileSize / 2, py + currentTileSize / 2, 4 * zoom, 0, Math.PI * 2);
        ctx.fill();

        // Label
        if (zoom > 0.5) {
            ctx.fillStyle = 'white';
            ctx.font = `${10 * zoom}px Outfit`;
            ctx.fillText(index + 1, px + currentTileSize / 2 + 6, py + currentTileSize / 2 - 6);
        }
    });
}

function updatePointsList() {
    pointsList.innerHTML = '';
    pointCountDisplay.textContent = points.length;

    points.forEach((p, index) => {
        const div = document.createElement('div');
        div.className = 'point-item';
        div.innerHTML = `
            <div class="point-info">
                <span class="point-coords">#${index + 1}: (${p.x}, ${p.y})</span>
            </div>
            <div class="point-actions">
                <button class="icon-btn" onclick="editPoint(${index})">📝</button>
                <button class="icon-btn danger" onclick="removePoint(${index})">✕</button>
            </div>
        `;
        pointsList.appendChild(div);
    });
}

window.editPoint = (index) => {
    selectedPointIndex = index;
    const p = points[index];
    document.getElementById('modal-point-info').textContent = `Point #${index + 1} at (${p.x}, ${p.y})`;
    document.getElementById('point-cmds').value = p.cmds;
    document.getElementById('edit-modal').classList.remove('hidden');
};

window.removePoint = (index) => {
    points.splice(index, 1);
    updatePointsList();
    render();
};

function savePointCmds() {
    if (selectedPointIndex !== null) {
        points[selectedPointIndex].cmds = document.getElementById('point-cmds').value;
    }
    closeModal();
}

function closeModal() {
    document.getElementById('edit-modal').classList.add('hidden');
    selectedPointIndex = null;
}

function clearAll() {
    if (confirm('Are you sure you want to clear all points?')) {
        points = [];
        updatePointsList();
        render();
    }
}

async function generateConfigs() {
    if (points.length === 0) {
        alert('Please add some points first!');
        return;
    }

    const folder = folderNameInput.value || 'aether';
    const bindKey = bindKeyInput.value || 'x';
    const defaultCmds = defaultCmdsInput.value;

    const startCmd = `rcon exec ${folder}/1.cfg`;
    document.getElementById('start-cmd').textContent = startCmd;
    document.getElementById('success-overlay').classList.remove('hidden');
}

function copyStartCmd() {
    const text = document.getElementById('start-cmd').textContent;
    navigator.clipboard.writeText(text);
    const btn = document.getElementById('copy-start-cmd');
    btn.textContent = 'Copied!';
    setTimeout(() => btn.textContent = 'Copy', 2000);
}

async function downloadZip() {
    const zip = new JSZip();
    const folder = folderNameInput.value || 'aether';
    const bindKey = bindKeyInput.value || 'x';
    const defaultCmds = defaultCmdsInput.value;

    points.forEach((p, index) => {
        const nextId = (index + 1) % points.length + 1;
        const configContent = [
            `# Point ${index + 1}`,
            `rcon sv_teleport ${p.x} ${p.y}`,
            defaultCmds,
            p.cmds,
            `bind ${bindKey} "exec ${folder}/${nextId}.cfg"`,
            `exec ${folder}/point_${index + 1}_custom.cfg` // Optional hook
        ].filter(line => line).join('\n');

        zip.file(`${folder}/${index + 1}.cfg`, configContent);
    });

    const blob = await zip.generateAsync({ type: 'blob' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `${folder}-configs.zip`;
    a.click();
}

function exportState() {
    const data = {
        points,
        settings: {
            bindKey: bindKeyInput.value,
            folderName: folderNameInput.value,
            defaultCmds: defaultCmdsInput.value
        }
    };
    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'aether-state.json';
    a.click();
}

init();
