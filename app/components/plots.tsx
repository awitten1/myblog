import React from 'react'
import { readFile } from "fs/promises"
import { TestSvgComponent } from "./client/graph"
import { DuckDBConnection } from '@duckdb/node-api';
import * as d3 from "d3";
import { InteractiveLine } from "./interactive_line"

const ROOT_DIR = process.env.ROOT_DIR || process.cwd()

export async function DrawSvgLine({
  height,
  width,
  alg,
  strokeColor
}: {
  height: number;
  width: number;
  alg: string;
  strokeColor: string;
}) {
  const connection = await DuckDBConnection.create();

  function template_query(file: string) {
    return `create or replace temp table results as
        select distinct on(name)
          cycles,instructions,
          substr(name, position('_' in name)+1,
            position('/' in name)-position('_' in name)-1) as algorithm,
          substr(name,position('/' in name)+1,
            length(name)-position('/' in name))::int as input_size
        from '${ROOT_DIR}/${file}'
        where cycles is not null and input_size < (1 << 10)
        `
  }
  const query = template_query('/code/binary-search/results/amdzen5/cycles.csv',)

  await connection.run(query)

  let reader = await connection.runAndReadAll(`select max(input_size) max_input_size,
    max(cycles) max_cycle_count from results`)
  let { max_input_size, max_cycle_count } = reader.getRowObjects()[0];

  const x = d3.scaleLinear([0, max_input_size], [0, width]);
  const y = d3.scaleLinear([0, max_cycle_count], [0, height]);

  reader = await connection.runAndReadAll(
    `select * from results
    where algorithm = '${alg}'
    order by input_size asc`);
  const rows = reader.getRowObjects();

  const points = rows.map((row: any) => ({
    x: x(row.input_size),
    y: height - y(row.cycles),
    input_size: row.input_size,
    cycles: row.cycles
  }));

  const labelMap: Record<string, string> = {
    'BinarySearch': 'Binary Search',
    'LinearSearch': 'Linear Search'
  };

  return (
    <InteractiveLine
      id={alg}
      label={labelMap[alg] || alg}
      points={points}
      strokeColor={strokeColor}
    />
  )
}