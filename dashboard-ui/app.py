import os
import json
import glob
from flask import Flask, render_template, jsonify, request
from datetime import datetime, timezone

app = Flask(__name__)

REPORTS_DIR = os.path.join(os.getcwd(), 'reports')

def get_all_reports():
    """Scans and parses all valid JSON reports with strict schema validation."""
    reports = []
    if not os.path.exists(REPORTS_DIR): return []
    
    files = glob.glob(os.path.join(REPORTS_DIR, "*.json"))
    for file_path in files:
        try:
            with open(file_path, 'r') as f:
                data = json.load(f)
                if not isinstance(data, dict) or 'metadata' not in data: continue
                data['_filename'] = os.path.basename(file_path)
                reports.append(data)
        except Exception:
            continue
    
    # Sort by timestamp descending
    reports.sort(key=lambda x: x.get('metadata', {}).get('timestamp', ''), reverse=True)
    return reports

def calculate_metrics(all_reports, selected_project=None):
    """Aggregates security telemetry and prepares multi-build history."""
    total_builds = len(all_reports)
    if total_builds == 0:
        return {
            "total_builds": 0, "total_criticals": 0, "total_warnings": 0,
            "safety_percentage": 0, "latest_issues": 0, "latest_crit_count": 0,
            "latest_warn_count": 0, "project_history": [], "all_reports": []
        }

    # Totals
    total_criticals = 0
    total_warnings = 0
    for r in all_reports:
        issues = r.get('issues', [])
        total_criticals += sum(1 for i in issues if i.get('severity') == 'Critical')
        total_warnings += sum(1 for i in issues if i.get('severity') == 'Warning')

    secure_builds = sum(1 for r in all_reports if r.get('summary', {}).get('status') == 'Secure')
    safety_percentage = round((secure_builds / total_builds * 100), 1)
    
    # Target Project Logic
    target_project = selected_project or all_reports[0]['metadata']['job_name']
    project_filtered_reports = [r for r in all_reports if r.get('metadata', {}).get('job_name') == target_project]
    project_filtered_reports.sort(key=lambda x: x.get('metadata', {}).get('timestamp', ''))

    # Prepare project history for the graph
    project_history = []
    for r in project_filtered_reports:
        issues = r.get('issues', [])
        project_history.append({
            "t": r['metadata']['timestamp'],
            "total": r.get('summary', {}).get('total_issues', 0),
            "crit": sum(1 for i in issues if i.get('severity') == 'Critical'),
            "warn": sum(1 for i in issues if i.get('severity') == 'Warning'),
            "bid": r['metadata'].get('build_id', 'N/A')
        })


    # Prepare data for the frontend state (Build History + Violations)
    client_reports = []
    for r in all_reports:
        issues = r.get('issues', [])
        client_reports.append({
            "build_id": r['metadata'].get('build_id'),
            "job_name": r['metadata'].get('job_name'),
            "status": r.get('summary', {}).get('status'),
            "timestamp": r.get('summary', {}).get('timestamp'),
            "issues": issues,
            "crit_count": sum(1 for i in issues if i.get('severity') == 'Critical'),
            "warn_count": sum(1 for i in issues if i.get('severity') == 'Warning')
        })

    return {
        "total_builds": total_builds,
        "total_criticals": total_criticals,
        "total_warnings": total_warnings,
        "safety_percentage": safety_percentage,
        "project_history": project_history,
        "all_reports": client_reports # Contains full violation detail
    }

@app.route('/')
def index():
    all_reports = get_all_reports()
    projects = sorted(list(set(r.get('metadata', {}).get('job_name') for r in all_reports if r.get('metadata'))))
    selected_project = request.args.get('project', projects[0] if projects else None)
    metrics = calculate_metrics(all_reports, selected_project)
    return render_template('dashboard.html', projects=projects, selected_project=selected_project, **metrics)

@app.route('/api/data')
def api_data():
    all_reports = get_all_reports()
    projects = sorted(list(set(r.get('metadata', {}).get('job_name') for r in all_reports if r.get('metadata'))))
    selected_project = request.args.get('project', projects[0] if projects else None)
    metrics = calculate_metrics(all_reports, selected_project)
    return jsonify({**metrics, "projects": projects})

if __name__ == '__main__':
    app.run(debug=True, port=5000)
