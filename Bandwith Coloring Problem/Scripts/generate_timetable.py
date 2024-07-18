import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys
import os
import ast


def generate_timetable(tmp_path):
    directory, filename = os.path.split(tmp_path)
    filename, _ = os.path.splitext(filename)
    exams = []
    days = []
    start_h = []
    end_h = []
    with open(tmp_path, 'r') as f:
        line = f.readline()
        exams = ast.literal_eval(line.strip())
        line = f.readline()
        days = ast.literal_eval(line.strip())
        line = f.readline()
        start_h = ast.literal_eval(line.strip())
        line = f.readline()
        end_h = ast.literal_eval(line.strip())

    exam_data = {
        'Exam': exams, 
        'Day': days,
        'Start Time': start_h,
        'End Time': end_h
    }

    df = pd.DataFrame(exam_data)

    days = list(range(1, 16))
    timeslots = pd.date_range("08:00", "18:00", freq="2h").time

    day_names = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri']
    weeks = ['Week 1'] * 5 + ['Week 2'] * 5 + ['Week 3'] * 5
    day_labels = [day_names[i % len(day_names)] for i in range(len(days))]

    timetable_dict = {f"Week {i + 1}": {day: {time: [] for time in timeslots} for day in day_names} for i in range(3)}

    for i, row in df.iterrows():
        week = f"Week {((row['Day'] - 1) // 5) + 1}"
        day = day_names[(row['Day'] - 1) % len(day_names)]
        start_time = pd.to_datetime(row['Start Time']).time()
        end_time = pd.to_datetime(row['End Time']).time()
        for time in timeslots:
            if start_time <= time < end_time:
                timetable_dict[week][day][time].append(row['Exam'])

    multi_index_cols = pd.MultiIndex.from_tuples([(week, day) for week in timetable_dict for day in timetable_dict[week]])
    timetable_data = {col: [] for col in multi_index_cols}

    for time in timeslots:
        for week in timetable_dict:
            for day in timetable_dict[week]:
                timetable_data[(week, day)].append('\n'.join(timetable_dict[week][day][time]) if timetable_dict[week][day][time] else '')

    timetable_df = pd.DataFrame(timetable_data, index=[t.strftime('%H:%M') for t in timeslots])

    header_weeks = [''] + [f'Week {i // 5 + 1}' for i in range(len(days))]
    header_days = ['Time'] + day_labels

    fig, ax = plt.subplots(figsize=(15, 8))
    ax.axis('tight')
    ax.axis('off')

    header_cells = np.zeros((2, timetable_df.shape[1] + 1), dtype=object)
    header_cells[0, 0] = ''
    header_cells[1, 0] = 'Time'
    for i in range(len(days)):
        header_cells[0, i + 1] = header_weeks[i + 1]
        header_cells[1, i + 1] = header_days[i + 1]

    table_data = np.vstack([header_cells, np.array([timetable_df.index] + timetable_df.T.values.tolist()).T])

    table = ax.table(cellText=table_data,
                    cellLoc='center', loc='center')

    table.auto_set_font_size(False)
    table.set_fontsize(15) 

    nrows, ncols = timetable_df.shape
    cell_height = 1 / (nrows + 3) 
    cell_width = 1 / (ncols + 1)

    for key, cell in table.get_celld().items():
        cell.set_height(cell_height)
        cell.set_width(cell_width)
        cell.set_text_props(va='center', ha='center', wrap=True)

    for i in range(1, len(day_labels) + 1, 5):
        table[0, i].visible_edges = 'LT'
        table[0, i + 4].visible_edges = 'TR'
        for j in range(i + 1, i + 4):
            table[0, j].visible_edges = 'BT'
        
        for j in range(i, i + 5):
            if j == i + 2:
                continue

            table[0, j].get_text().set_text('')


    fig.set_size_inches(ncols * cell_width * 15, (nrows + 2) * cell_height * 15)

    plt.savefig(directory + '\\' + filename + '.png', format="png")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python draw_graph.py <edges> <node_values> <name>")
        sys.exit(1)

    tmp_path = sys.argv[1]
    generate_timetable(tmp_path)

