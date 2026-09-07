(() => {
  "use strict";

  const completedKey = "rtc-practice-completed-days";
  const state = { exercises: [], current: null, activeTab: "readme", completed: loadCompleted() };
  const $ = (id) => document.getElementById(id);

  function loadCompleted() {
    try {
      const value = JSON.parse(localStorage.getItem(completedKey) || "[]");
      return new Set(value.filter((day) => Number.isInteger(day) && day >= 1 && day <= 30));
    } catch (_) { return new Set(); }
  }

  function persistCompleted() { localStorage.setItem(completedKey, JSON.stringify([...state.completed].sort((a, b) => a - b))); }
  function setStatus(text, isError = false) { const node = $("status"); node.textContent = text; node.classList.toggle("error", isError); }
  function dayLabel(day) { return String(day).padStart(2, "0"); }

  function renderProgress() { $("progress").textContent = `${state.completed.size} / 30 已标记完成`; }
  function renderList() {
    const list = $("exercise-list"); list.replaceChildren();
    state.exercises.forEach((exercise) => {
      const button = document.createElement("button"); button.className = "exercise"; button.type = "button"; button.dataset.day = exercise.day;
      if (state.current && state.current.day === exercise.day) button.classList.add("active");
      const day = document.createElement("span"); day.className = "day"; day.textContent = dayLabel(exercise.day);
      const title = document.createElement("span"); title.textContent = exercise.title;
      button.append(day, title);
      if (state.completed.has(exercise.day)) { const done = document.createElement("span"); done.className = "complete"; done.textContent = "✓"; button.append(done); }
      button.addEventListener("click", () => loadExercise(exercise.day)); list.append(button);
    });
  }

  function renderDetail() {
    const detail = state.current; if (!detail) return;
    $("topic").textContent = `${detail.cpp_topics} · ${detail.rtc_topics}`; $("title").textContent = `Day ${dayLabel(detail.day)} · ${detail.title}`; $("summary").textContent = detail.summary;
    $("check").disabled = false; $("run").disabled = false; $("copy-path").disabled = false; $("mark-complete").disabled = false;
    $("mark-complete").textContent = state.completed.has(detail.day) ? "取消完成标记" : "标记为完成";
    renderContent(); renderList();
  }

  function renderContent() {
    if (!state.current) return;
    const content = $("content");
    const isCpp = state.activeTab === "header" || state.activeTab === "source";
    content.textContent = state.current[state.activeTab];
    content.className = isCpp ? "language-cpp" : "";
    if (isCpp && window.Prism) window.Prism.highlightElement(content);
    document.querySelectorAll(".tab").forEach((tab) => tab.classList.toggle("active", tab.dataset.tab === state.activeTab));
  }

  async function loadExercise(day) {
    $("check").disabled = true; $("run").disabled = true; setStatus(`正在读取 Day ${dayLabel(day)}…`); $("result").classList.add("hidden");
    try {
      const response = await fetch(`/api/exercises/${day}`, { cache: "no-store" }); const detail = await response.json();
      if (!response.ok) throw new Error(detail.error || "无法读取练习");
      state.current = detail; state.activeTab = "readme"; renderDetail(); setStatus(`在本地编辑器完成 ${detail.source_path}，保存后点击运行。`);
    } catch (error) { setStatus(`加载失败：${error.message}`, true); }
  }

  async function executeCurrent(action) {
    if (!state.current) return;
    const day = state.current.day;
    const button = action === "check" ? $("check") : $("run");
    button.disabled = true;
    setStatus(action === "check" ? `正在构建并运行 Day ${dayLabel(day)} 验收测试…` : `正在构建并运行 Day ${dayLabel(day)} run() 示例…`);
    try {
      const response = await fetch(`/api/exercises/${day}/${action}`, { method: "POST" }); const result = await response.json();
      if (!response.ok) throw new Error(result.error || "运行请求失败");
      $("result").classList.remove("hidden"); const badge = $("result-state"); badge.textContent = result.state; badge.className = `badge ${result.state}`; $("elapsed").textContent = `${result.elapsed_ms} ms`; $("result-output").textContent = result.output || "(没有输出)";
      setStatus(action === "check" && result.state === "passed" ? "验收通过。" : `${action === "check" ? "验收测试" : "run()"} 已完成，请查看输出。`, result.state === "failed" || result.state === "build_error" || result.state === "timed_out");
    } catch (error) { setStatus(`运行失败：${error.message}`, true); } finally { button.disabled = false; }
  }

  async function copySourcePath() {
    if (!state.current) return;
    try { await navigator.clipboard.writeText(state.current.source_path); setStatus(`已复制 ${state.current.source_path}`); }
    catch (_) { setStatus(`请在编辑器中打开：${state.current.source_path}`); }
  }

  function toggleCompletion() { if (!state.current) return; const day = state.current.day; state.completed.has(day) ? state.completed.delete(day) : state.completed.add(day); persistCompleted(); renderProgress(); renderDetail(); }

  async function start() {
    $("check").addEventListener("click", () => executeCurrent("check")); $("run").addEventListener("click", () => executeCurrent("run")); $("copy-path").addEventListener("click", copySourcePath); $("mark-complete").addEventListener("click", toggleCompletion);
    document.querySelectorAll(".tab").forEach((tab) => tab.addEventListener("click", () => { state.activeTab = tab.dataset.tab; renderContent(); }));
    try { const response = await fetch("/api/exercises", { cache: "no-store" }); const data = await response.json(); if (!response.ok) throw new Error(data.error || "无法加载练习目录"); state.exercises = data.exercises; renderProgress(); await loadExercise(1); }
    catch (error) { setStatus(`服务不可用：${error.message}`, true); }
  }

  void start();
})();
