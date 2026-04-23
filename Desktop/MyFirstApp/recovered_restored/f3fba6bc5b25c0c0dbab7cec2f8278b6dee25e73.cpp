class TodoApp {
    constructor() {
        this.tasks = this.loadTasks();
        this.currentFilter = {
            priority: 'all',
            category: 'all',
            status: 'all'
        };
        this.currentSort = 'date-created';
        this.editingTaskId = null;
        this.reminderIntervals = [];

        // Cloud sync settings
        this.cloudSyncEnabled = false;
        this.supabase = null;

        this.init();
    }

    init() {
        this.bindEvents();
        this.render();
        this.updateStats();
    }

    bindEvents() {
        // 添加任务按钮
        document.getElementById('add-task-btn').addEventListener('click', () => {
            this.openModal();
        });

        // 模态框相关事件
        const modal = document.getElementById('task-modal');
        const closeBtn = document.querySelector('.close');
        const cancelBtn = document.getElementById('cancel-btn');
        const taskForm = document.getElementById('task-form');

        closeBtn.addEventListener('click', () => this.closeModal());
        cancelBtn.addEventListener('click', () => this.closeModal());
        modal.addEventListener('click', (e) => {
            if (e.target === modal) this.closeModal();
        });

        taskForm.addEventListener('submit', (e) => {
            e.preventDefault();
            this.saveTask();
        });

        // 筛选器事件
        document.getElementById('priority-filter').addEventListener('change', (e) => {
            this.currentFilter.priority = e.target.value;
            this.render();
        });

        document.getElementById('category-filter').addEventListener('change', (e) => {
            this.currentFilter.category = e.target.value;
            this.render();
        });

        document.getElementById('status-filter').addEventListener('change', (e) => {
            this.currentFilter.status = e.target.value;
            this.render();
        });

        // 排序事件
        document.getElementById('sort-by').addEventListener('change', (e) => {
            this.currentSort = e.target.value;
            this.render();
        });
    }

    loadTasks() {
        const saved = localStorage.getItem('todo-tasks');
        return saved ? JSON.parse(saved) : [];
    }

    saveTasks() {
        localStorage.setItem('todo-tasks', JSON.stringify(this.tasks));
    }

    generateId() {
        return Date.now().toString(36) + Math.random().toString(36).substr(2);
    }

    openModal(taskId = null) {
        const modal = document.getElementById('task-modal');
        const modalTitle = document.getElementById('modal-title');

        if (taskId) {
            modalTitle.textContent = '编辑任务';
            this.editingTaskId = taskId;
            this.populateForm(taskId);
        } else {
            modalTitle.textContent = '添加新任务';
            this.editingTaskId = null;
            document.getElementById('task-form').reset();
        }

        modal.style.display = 'block';
    }

    closeModal() {
        document.getElementById('task-modal').style.display = 'none';
    }

    populateForm(taskId) {
        const task = this.tasks.find(t => t.id === taskId);
        if (task) {
            document.getElementById('task-title').value = task.title;
            document.getElementById('task-description').value = task.description || '';
            document.getElementById('task-priority').value = task.priority;
            document.getElementById('task-category').value = task.category || '';
            document.getElementById('task-due-date').value = task.dueDate || '';
        }
    }

    saveTask() {
        const formData = {
            title: document.getElementById('task-title').value,
            description: document.getElementById('task-description').value,
            priority: document.getElementById('task-priority').value,
            category: document.getElementById('task-category').value,
            dueDate: document.getElementById('task-due-date').value
        };

        if (!formData.title.trim()) {
            alert('请输入任务标题');
            return;
        }

        if (this.editingTaskId) {
            // 编辑现有任务
            const task = this.tasks.find(t => t.id === this.editingTaskId);
            if (task) {
                Object.assign(task, formData);
                task.updatedAt = new Date().toISOString();
            }
        } else {
            // 添加新任务
            const newTask = {
                id: this.generateId(),
                ...formData,
                completed: false,
                createdAt: new Date().toISOString(),
                updatedAt: new Date().toISOString()
            };
            this.tasks.push(newTask);
        }

        this.saveTasks();
        this.closeModal();
        this.render();
        this.updateStats();
    }

    toggleTask(taskId) {
        const task = this.tasks.find(t => t.id === taskId);
        if (task) {
            task.completed = !task.completed;
            task.updatedAt = new Date().toISOString();
            this.saveTasks();
            this.render();
            this.updateStats();
        }
    }

    editTask(taskId) {
        this.openModal(taskId);
    }

    deleteTask(taskId) {
        if (confirm('确定要删除这个任务吗？')) {
            this.tasks = this.tasks.filter(t => t.id !== taskId);
            this.saveTasks();
            this.render();
            this.updateStats();
        }
    }

    getFilteredAndSortedTasks() {
        let filtered = [...this.tasks];

        // 应用筛选
        if (this.currentFilter.priority !== 'all') {
            filtered = filtered.filter(t => t.priority === this.currentFilter.priority);
        }

        if (this.currentFilter.category !== 'all') {
            filtered = filtered.filter(t => t.category === this.currentFilter.category);
        }

        if (this.currentFilter.status !== 'all') {
            filtered = filtered.filter(t => {
                if (this.currentFilter.status === 'completed') return t.completed;
                if (this.currentFilter.status === 'pending') return !t.completed;
                return true;
            });
        }

        // 应用排序
        filtered.sort((a, b) => {
            switch (this.currentSort) {
                case 'date-created':
                    return new Date(b.createdAt) - new Date(a.createdAt);
                case 'due-date':
                    if (!a.dueDate) return 1;
                    if (!b.dueDate) return -1;
                    return new Date(a.dueDate) - new Date(b.dueDate);
                case 'priority':
                    const priorityOrder = { high: 3, medium: 2, low: 1 };
                    return priorityOrder[b.priority] - priorityOrder[a.priority];
                default:
                    return 0;
            }
        });

        return filtered;
    }

    render() {
        const taskList = document.getElementById('task-list');
        const filteredTasks = this.getFilteredAndSortedTasks();

        // 更新分类筛选器
        this.updateCategoryFilter();

        if (filteredTasks.length === 0) {
            taskList.innerHTML = `
                <div class="empty-state">
                    <div class="empty-state-icon">📝</div>
                    <h3>暂无任务</h3>
                    <p>点击"添加新任务"开始创建你的第一个任务吧！</p>
                </div>
            `;
            return;
        }

        taskList.innerHTML = filteredTasks.map(task => this.renderTask(task)).join('');

        // 绑定任务操作事件
        filteredTasks.forEach(task => {
            const taskElement = document.querySelector(`[data-task-id="${task.id}"]`);
            if (taskElement) {
                taskElement.querySelector('.task-checkbox').addEventListener('change', () => {
                    this.toggleTask(task.id);
                });

                taskElement.querySelector('.btn-edit').addEventListener('click', () => {
                    this.editTask(task.id);
                });

                taskElement.querySelector('.btn-delete').addEventListener('click', () => {
                    this.deleteTask(task.id);
                });
            }
        });
    }

    renderTask(task) {
        const priorityClass = `priority-${task.priority}`;
        const isOverdue = task.dueDate && !task.completed && new Date(task.dueDate) < new Date();
        const dueDateClass = isOverdue ? 'overdue' : '';
        const dueDateText = task.dueDate ? new Date(task.dueDate).toLocaleDateString('zh-CN') : '未设置';

        return `
            <div class="task-item ${task.completed ? 'completed' : ''}" data-task-id="${task.id}">
                <input type="checkbox" class="task-checkbox" ${task.completed ? 'checked' : ''}>
                <div class="task-priority ${priorityClass}"></div>
                <div class="task-content">
                    <h4>${this.escapeHtml(task.title)}</h4>
                    ${task.description ? `<p class="task-description">${this.escapeHtml(task.description)}</p>` : ''}
                    <div class="task-meta">
                        <span class="task-category">${task.category ? this.escapeHtml(task.category) : '未分类'}</span>
                        <span class="task-due-date ${dueDateClass}" title="${isOverdue ? '已过期' : '截止日期'}">
                            ${isOverdue ? '⚠️ ' : '📅 '}${dueDateText}
                        </span>
                    </div>
                </div>
                <div class="task-actions">
                    <button class="btn-icon btn-edit" title="编辑">✏️</button>
                    <button class="btn-icon btn-delete" title="删除">🗑️</button>
                </div>
            </div>
        `;
    }

    updateCategoryFilter() {
        const categoryFilter = document.getElementById('category-filter');
        const categories = [...new Set(this.tasks.map(t => t.category).filter(c => c))].sort();

        // 保存当前选中的分类
        const currentCategory = this.currentFilter.category;

        // 重新生成分类选项
        const categoryOptions = ['<option value="all">全部分类</option>']
            .concat(categories.map(cat => `<option value="${this.escapeHtml(cat)}">${this.escapeHtml(cat)}</option>`))
            .join('');

        categoryFilter.innerHTML = categoryOptions;

        // 恢复之前选中的分类
        if (categories.includes(currentCategory) || currentCategory === 'all') {
            categoryFilter.value = currentCategory;
        }
    }

    updateStats() {
        const total = this.tasks.length;
        const completed = this.tasks.filter(t => t.completed).length;

        document.getElementById('total-tasks').textContent = `${total} 个任务`;
        document.getElementById('completed-tasks').textContent = `${completed} 已完成`;
    }

    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}

// 应用初始化
document.addEventListener('DOMContentLoaded', () => {
    new TodoApp();
});