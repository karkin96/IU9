from lib import save_to_csv
from random import choice, uniform
from time import sleep

CSV_HEADERS = [
    'substance',
    'length',
    'width',
    'height',
    'expansivity',
    'compression',
    'step_coefficient',
    'rot_eps',
    'trans_eps',
    'lj_epsilon'
]

MOLS_ROOT = '../resources/'

MOLS_EPS = {
    'H2O': 0.6678,
    'NH3': 0.3085,
    'CHCl3': 0.3059,
    'CH4O': 0.3593,
    'C2H5Cl': 0.3645
}

STEP_COEF = [0.5, 0.25, 0.33, 0.2, 0.16, 0.143, 0.125, 0.11, 0.1] 
EXPANS = [1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.3] 
COMPR = [0.5, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 1.0]
ROT_EPS = [2.0, 1.0, 0.1, 0.01, 0.001]
TRANS_EPS = [0.1, 0.05, 0.01, 0.001, 0.0001]

CHOICES_SET = set()


def get_new_random_choice():
    choice_tuple = (choice(EXPANS), choice(COMPR), choice(STEP_COEF), choice(ROT_EPS), choice(TRANS_EPS))
    while choice_tuple in CHOICES_SET:
        choice_tuple = (choice(STEP_COEF), choice(EXPANS), choice(COMPR), choice(ROT_EPS), choice(TRANS_EPS))

    CHOICES_SET.add(choice_tuple)

    return (choice_tuple[0], choice_tuple[1], choice_tuple[2], choice_tuple[3], choice_tuple[4])


def generate_random_tests(substance, count):
    tests = []
    while len(CHOICES_SET) < count:
        choice = get_new_random_choice()

        length = uniform(4.0, 30.0)
        width = uniform(4.0, 30.0)
        height = uniform(4.0, 30.0)

        params_lst = [substance, length, width, height]
        params_lst.extend(list(choice))
        params_lst.append(MOLS_EPS[substance])

        tests.append(params_lst)

    return tests


tests_all = []
for substance in MOLS_EPS:
    tests = generate_random_tests(substance, 5)
    tests_all.extend(tests)
    CHOICES_SET.clear()

save_to_csv('rand_tests_3.csv', CSV_HEADERS, tests_all) 

