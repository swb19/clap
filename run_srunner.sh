source devel/setup.bash
python ${ROOT_SCENARIO_RUNNER}/srunner/challenge/challenge_evaluator_routes.py --scenarios=${ROOT_SCENARIO_RUNNER}/srunner/challenge/all_towns_traffic_scenarios1_3_4.json --routes=${ROOT_SCENARIO_RUNNER}/srunner/challenge/routes_training.xml --debug=0 --agent=${TEAM_AGENT} --config=${TEAM_CONFIG}
