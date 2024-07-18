import json
import random

def generate_random_numbers_with_sum(sum_value, num_parts):
    total_positions = sum_value + (num_parts - 1)
    
    bars = sorted(random.sample(range(total_positions), num_parts - 1))
    numbers = []
    current_position = 0
    for bar in bars:
        numbers.append(bar - current_position)
        current_position = bar + 1
    numbers.append(total_positions - current_position)
    
    return numbers

def generate_distribution():
    no_students = random.randint(300, 400)
    no_exams = random.randint(6, 10)
    no_optional_exams = random.randint(1, 5)
    no_mandatory_exams = no_exams - no_optional_exams

    numbers = generate_random_numbers_with_sum(no_mandatory_exams, 3)

    no_mandatory_exams = [
        numbers[0],
        numbers[1],
        numbers[2]
    ]

    numbers = generate_random_numbers_with_sum(no_optional_exams, 3)


    no_optional_packs = [
        numbers[0],
        numbers[1],
        numbers[2]
    ]

    it = 0

    exams = {}
    labels = ['easy', 'medium', 'hard']
    for index in range(len(labels)):
        for _ in range(no_mandatory_exams[index]):
            exams[it] = [f'e{it}', labels[index]]
            it = it + 1

    packs = {}
    pack_it = 0
    for index in range(len(labels)):
        for _ in range(no_optional_packs[index]):
            opt_exams = []
            for _ in range(4):
                opt_exams.append(f'e{it}')
                it = it + 1

            packs[pack_it] = [opt_exams, labels[index]]
            pack_it = pack_it + 1

    stud_exams = {}
    opt_occupation = {}
    maxim_space_for_optional = no_students // 3
    for stud_id in range(no_students):
        curr_exams = []
        for mandatory_exam in exams.keys():
            curr_exams.append(exams[mandatory_exam][0])
        
        for optional_exam in packs.keys():
            chose = packs[optional_exam][0]
            for ex in chose:
                if ex not in opt_occupation:
                    opt_occupation[ex] = 0
            
            while True:
                rand = random.randint(0, 3)
                if opt_occupation[chose[rand]] < maxim_space_for_optional:
                    opt_occupation[chose[rand]] = opt_occupation[chose[rand]] + 1
                    curr_exams.append(chose[rand])
                    break
        
        stud_exams[stud_id] = curr_exams

    final_result = {
        'mandatory_exams' : exams,
        'optional_packs' : packs,
        'students': stud_exams
    }
                
    return final_result


if __name__ == '__main__':

    path = R'C:\Users\lucian.isac\source\repos\Bandwith Coloring Problem\Bandwith Coloring Problem\Instances\UETT_Instances\generated_json'

    no_exams_gen = 10
    for it in range(no_exams_gen):
        with open(path + f'\exam_data_{it}.json', 'w') as json_file:
            json.dump(generate_distribution(), json_file, indent=4)