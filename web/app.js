// JavaScript source code
let token = localStorage.getItem("token") || "";
const MAX_UPLOAD_SIZE = 100 * 1024 * 1024;

function formatFileSize(size) {
    if (size < 1024) {
        return size + " B";
    }

    if (size < 1024 * 1024) {
        return (size / 1024).toFixed(2) + " KB";
    }

    if (size < 1024 * 1024 * 1024) {
        return (size / 1024 / 1024).toFixed(2) + " MB";
    }

    return (size / 1024 / 1024 / 1024).toFixed(2) + " GB";
}

function updateSelectedFileInfo() {
    const fileInput = document.getElementById("fileInput");
    const selectedFileInfo = document.getElementById("selectedFileInfo");

    if (!fileInput.files || fileInput.files.length === 0) {
        selectedFileInfo.textContent = "未选择文件";
        return;
    }

    const file = fileInput.files[0];

    selectedFileInfo.textContent =
        "已选择：" + file.name + "，大小：" + formatFileSize(file.size);

    if (file.size > MAX_UPLOAD_SIZE) {
        selectedFileInfo.textContent += "，超过 100 MB 限制，无法上传";
    }
}

function updateUIState() {
    const isLoggedIn = !!token;

    const registerSection = document.getElementById("registerSection");
    const loginSection = document.getElementById("loginSection");
    const uploadSection = document.getElementById("uploadSection");
    const fileSection = document.getElementById("fileSection");
    const shareSection = document.getElementById("shareSection");
    const logSection = document.getElementById("logSection");
    const logoutBtn = document.getElementById("logoutBtn");

    if (isLoggedIn) {
        registerSection.classList.add("hidden");
        loginSection.classList.remove("hidden");
        uploadSection.classList.remove("hidden");
        fileSection.classList.remove("hidden");
        shareSection.classList.remove("hidden");
        logSection.classList.remove("hidden");
        logoutBtn.classList.remove("hidden");
    } else {
        registerSection.classList.remove("hidden");
        loginSection.classList.remove("hidden");
        uploadSection.classList.add("hidden");
        fileSection.classList.add("hidden");
        shareSection.classList.add("hidden");
        logSection.classList.add("hidden");
        logoutBtn.classList.add("hidden");
    }
}

window.onload = function () {
    updateTokenText();
    updateUIState();

    const fileInput = document.getElementById("fileInput");
    if (fileInput) {
        fileInput.addEventListener("change", updateSelectedFileInfo);
    }

    if (token) {
        loadFileList();
        loadShareList();
    }
};

function updateTokenText() {
    const tokenText = document.getElementById("tokenText");
    tokenText.textContent = token ? token : "未登录";
}

function log(data) {
    const logBox = document.getElementById("logBox");

    if (typeof data === "string") {
        logBox.textContent = data;
    } else {
        logBox.textContent = JSON.stringify(data, null, 4);
    }
}

function getAuthHeaders() {
    return {
        "Authorization": "Bearer " + token
    };
}

async function postJson(url, body, needAuth = false) {
    const headers = {
        "Content-Type": "application/json"
    };

    if (needAuth) {
        headers["Authorization"] = "Bearer " + token;
    }

    const response = await fetch(url, {
        method: "POST",
        headers: headers,
        body: JSON.stringify(body)
    });

    return await response.json();
}

async function getJson(url, needAuth = false) {
    const headers = {};

    if (needAuth) {
        headers["Authorization"] = "Bearer " + token;
    }

    const response = await fetch(url, {
        method: "GET",
        headers: headers
    });

    return await response.json();
}

async function registerUser() {
    const username = document.getElementById("registerUsername").value.trim();
    const password = document.getElementById("registerPassword").value;

    const result = await postJson("/api/register", {
        username: username,
        password: password
    });

    log(result);
}

async function loginUser() {
    const username = document.getElementById("loginUsername").value.trim();
    const password = document.getElementById("loginPassword").value;

    const result = await postJson("/api/login", {
        username: username,
        password: password
    });

    log(result);

    if (result.code === 0 && result.data && result.data.token) {
        token = result.data.token;
        localStorage.setItem("token", token);
        updateTokenText();
        updateUIState();
        await loadFileList();
        await loadShareList();
    }
}

async function checkToken() {
    if (!token) {
        log("请先登录");
        return;
    }

    const result = await getJson("/api/me", true);
    log(result);
}

async function logout() {
    if (!token) {
        log("当前未登录");
        return;
    }

    const response = await fetch("/api/logout", {
        method: "POST",
        headers: getAuthHeaders()
    });

    const result = await response.json();
    log(result);

    if (result.code === 0) {
        token = "";
        localStorage.removeItem("token");
        updateTokenText();
        updateUIState();
        document.getElementById("fileList").innerHTML = "\u6682\u65e0\u6587\u4ef6";
        document.getElementById("shareList").innerHTML = "\u6682\u65e0\u5206\u4eab";
    }
}

function uploadFile() {
    if (!token) {
        log("请先登录");
        return;
    }

    const fileInput = document.getElementById("fileInput");

    if (!fileInput.files || fileInput.files.length === 0) {
        log("请选择文件");
        return;
    }

    const file = fileInput.files[0];

    if (file.size > MAX_UPLOAD_SIZE) {
        log({
            code: 4010,
            message: "file too large",
            max_size: formatFileSize(MAX_UPLOAD_SIZE),
            current_size: formatFileSize(file.size)
        });
        return;
    }

    const formData = new FormData();
    formData.append("file", file);

    const progressWrapper = document.getElementById("uploadProgressWrapper");
    const progressBar = document.getElementById("uploadProgressBar");

    progressWrapper.classList.remove("hidden");
    progressBar.style.width = "0%";
    progressBar.textContent = "0%";

    const xhr = new XMLHttpRequest();

    xhr.open("POST", "/api/file/upload", true);
    xhr.setRequestHeader("Authorization", "Bearer " + token);

    xhr.upload.onprogress = function (event) {
        if (event.lengthComputable) {
            const percent = Math.round((event.loaded / event.total) * 100);
            progressBar.style.width = percent + "%";
            progressBar.textContent = percent + "%";
        }
    };

    xhr.onload = async function () {
        try {
            const result = JSON.parse(xhr.responseText);
            log(result);

            if (xhr.status >= 200 && xhr.status < 300 && result.code === 0) {
                fileInput.value = "";
                updateSelectedFileInfo();

                progressBar.style.width = "100%";
                progressBar.textContent = "100%";

                await loadFileList();
            }
        } catch (e) {
            log("上传完成，但响应解析失败");
        }
    };

    xhr.onerror = function () {
        log("上传失败，请检查服务器是否正常运行");
    };

    xhr.onloadend = function () {
        setTimeout(function () {
            progressWrapper.classList.add("hidden");
            progressBar.style.width = "0%";
            progressBar.textContent = "0%";
        }, 1200);
    };

    xhr.send(formData);
}

async function loadFileList() {
    if (!token) {
        log("请先登录");
        return;
    }

    const result = await getJson("/api/file/list", true);
    log(result);

    const fileList = document.getElementById("fileList");

    if (result.code !== 0) {
        fileList.innerHTML = "文件列表加载失败";
        return;
    }

    if (!result.data || result.data.length === 0) {
        fileList.innerHTML = "暂无文件";
        return;
    }

    fileList.innerHTML = "";

    result.data.forEach(file => {
        const item = document.createElement("div");
        item.className = "file-item";

        const info = document.createElement("div");
        info.className = "file-info";

        const name = document.createElement("div");
        name.className = "file-name";
        name.textContent = file.original_name;

        const meta = document.createElement("div");
        meta.className = "file-meta";
        meta.textContent =
            "文件ID：" + file.file_id +
            " | 大小：" + file.file_size + " bytes" +
            " | 上传时间：" + file.created_at;

        info.appendChild(name);
        info.appendChild(meta);

        const actions = document.createElement("div");
        actions.className = "file-actions";

        const downloadBtn = document.createElement("button");
        downloadBtn.textContent = "下载";
        downloadBtn.onclick = function () {
            downloadFile(file.file_id, file.original_name);
        };

        const shareBtn = document.createElement("button");
        shareBtn.textContent = "分享";
        shareBtn.className = "secondary";
        shareBtn.onclick = function () {
            shareFile(file.file_id);
        };

        const deleteBtn = document.createElement("button");
        deleteBtn.textContent = "删除";
        deleteBtn.className = "danger";
        deleteBtn.onclick = function () {
            deleteFile(file.file_id);
        };

        actions.appendChild(downloadBtn);
        actions.appendChild(shareBtn);
        actions.appendChild(deleteBtn);

        item.appendChild(info);
        item.appendChild(actions);

        fileList.appendChild(item);
    });
}

async function downloadFile(fileId, fileName) {
    if (!token) {
        log("请先登录");
        return;
    }

    const response = await fetch("/api/file/download?file_id=" + fileId, {
        method: "GET",
        headers: getAuthHeaders()
    });

    const contentType = response.headers.get("Content-Type") || "";

    if (contentType.includes("application/json")) {
        const result = await response.json();
        log(result);
        return;
    }

    const blob = await response.blob();
    const url = window.URL.createObjectURL(blob);

    const a = document.createElement("a");
    a.href = url;
    a.download = fileName || "download.bin";
    document.body.appendChild(a);
    a.click();
    a.remove();

    window.URL.revokeObjectURL(url);

    log("下载完成：" + fileName);
}

async function deleteFile(fileId) {
    if (!token) {
        log("请先登录");
        return;
    }

    if (!confirm("确定要删除这个文件吗？")) {
        return;
    }

    const result = await postJson("/api/file/delete", {
        file_id: fileId
    }, true);

    log(result);

    if (result.code === 0) {
        await loadFileList();
        await loadShareList();
    }
}

async function shareFile(fileId) {
    if (!token) {
        log("请先登录");
        return;
    }

    const result = await postJson("/api/file/share", {
        file_id: fileId
    }, true);

    log(result);

    if (result.code === 0 && result.data && result.data.share_url) {
        const shareUrl = result.data.share_url;

        try {
            await navigator.clipboard.writeText(shareUrl);
            alert("\u5206\u4eab\u94fe\u63a5\u5df2\u590d\u5236\uff1a\n" + shareUrl);
        } catch (e) {
            alert("\u5206\u4eab\u94fe\u63a5\uff1a\n" + shareUrl);
        }

        await loadShareList();
    }
}

async function copyShareUrl(shareUrl) {
    try {
        await navigator.clipboard.writeText(shareUrl);
        alert("\u5206\u4eab\u94fe\u63a5\u5df2\u590d\u5236\uff1a\n" + shareUrl);
    } catch (e) {
        alert("\u5206\u4eab\u94fe\u63a5\uff1a\n" + shareUrl);
    }
}

async function loadShareList() {
    if (!token) {
        log("\u8bf7\u5148\u767b\u5f55");
        return;
    }

    const result = await getJson("/api/share/list", true);
    log(result);

    const shareList = document.getElementById("shareList");

    if (result.code !== 0) {
        shareList.innerHTML = "\u5206\u4eab\u5217\u8868\u52a0\u8f7d\u5931\u8d25";
        return;
    }

    if (!result.data || result.data.length === 0) {
        shareList.innerHTML = "\u6682\u65e0\u5206\u4eab";
        return;
    }

    shareList.innerHTML = "";

    result.data.forEach(share => {
        const item = document.createElement("div");
        item.className = "share-item";

        const info = document.createElement("div");
        info.className = "share-info";

        const name = document.createElement("div");
        name.className = "share-name";
        name.textContent = share.original_name || "\u672a\u77e5\u6587\u4ef6";

        const meta = document.createElement("div");
        meta.className = "share-meta";
        meta.textContent =
            "\u5206\u4eabID\uff1a" + share.share_id +
            " | \u6587\u4ef6ID\uff1a" + share.file_id +
            " | \u521b\u5efa\u65f6\u95f4\uff1a" + share.created_at +
            " | \u8fc7\u671f\u65f6\u95f4\uff1a" + share.expired_at;

        const url = document.createElement("div");
        url.className = "share-url";
        url.textContent = share.share_url;

        info.appendChild(name);
        info.appendChild(meta);
        info.appendChild(url);

        const actions = document.createElement("div");
        actions.className = "share-actions";

        const copyBtn = document.createElement("button");
        copyBtn.textContent = "\u590d\u5236\u94fe\u63a5";
        copyBtn.className = "secondary";
        copyBtn.onclick = function () {
            copyShareUrl(share.share_url);
        };

        const openBtn = document.createElement("button");
        openBtn.textContent = "\u6253\u5f00";
        openBtn.onclick = function () {
            window.open(share.share_url, "_blank");
        };

        const cancelBtn = document.createElement("button");
        cancelBtn.textContent = "\u53d6\u6d88\u5206\u4eab";
        cancelBtn.className = "danger";
        cancelBtn.onclick = function () {
            cancelShare(share.share_id);
        };

        actions.appendChild(copyBtn);
        actions.appendChild(openBtn);
        actions.appendChild(cancelBtn);

        item.appendChild(info);
        item.appendChild(actions);

        shareList.appendChild(item);
    });
}

async function cancelShare(shareId) {
    if (!token) {
        log("\u8bf7\u5148\u767b\u5f55");
        return;
    }

    if (!confirm("\u786e\u5b9a\u8981\u53d6\u6d88\u8fd9\u4e2a\u5206\u4eab\u5417\uff1f")) {
        return;
    }

    const result = await postJson("/api/share/cancel", {
        share_id: shareId
    }, true);

    log(result);

    if (result.code === 0) {
        await loadShareList();
    }
}