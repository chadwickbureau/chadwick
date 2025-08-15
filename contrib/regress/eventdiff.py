import pandas as pd


def check_row_count(df_a: pd.DataFrame, df_b: pd.DataFrame) -> None:
	if len(df_a) != len(df_b):
		print("Samples have different numbers of rows.")
		exit(1)
		

def check_column_count(df_a: pd.DataFrame, df_b: pd.DataFrame) -> None:
	if len(df_a.columns) != len(df_b.columns):
		print("Samples have different numbers of columns.")
		exit(1)


def main():
	df_chadwick = pd.read_csv("chadwick.csv", dtype=str)
	df_dware = pd.read_csv("dware.csv", header=None, dtype=str)
	
	check_row_count(df_chadwick, df_dware)
	check_column_count(df_chadwick, df_dware)
	
	df_dware.columns = df_chadwick.columns
	
	combined = pd.DataFrame({"chadwick": df_chadwick.stack(),
	                         "dware": df_dware.stack()})
	diffs = combined.query("chadwick != dware")
	if diffs.empty:
		print("No differences found")
		return
	
	diffs = (
		diffs
		.reset_index(level=1)
		.rename(columns={"level_1": "field"})
		.assign(**{"GAME_ID": df_chadwick["GAME_ID"],
		           "INN_CT": df_chadwick["INN_CT"],
		           "EVENT_TX": df_chadwick["EVENT_TX"]})
	)
	print(diffs.to_string())
	

if __name__ == "__main__":
	main()
