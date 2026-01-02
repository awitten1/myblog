
import { readFile } from "fs/promises"
import { TestSvgComponent } from "./client/graph"
import { DuckDBConnection } from '@duckdb/node-api';

const ROOT_DIR = process.env.ROOT_DIR

export async function Plot({ file }) {
  const connection = await DuckDBConnection.create();

  const reader = await connection.runAndReadAll(
    `select * from '${ROOT_DIR}/code/binary-search/results/amdzen5/cycles.csv'`
  );

  const rows = reader.getRowObjects();
  console.log(rows)

  return (
    <>
      <p>aasdf</p>
    </>
  )
}