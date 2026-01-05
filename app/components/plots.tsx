import React from 'react'
import { readFile } from "fs/promises"
import { LinePlotClient, Point, Line } from "./client/graph"
import { DuckDBConnection } from '@duckdb/node-api';
import * as d3 from "d3";
import './plot.css'

const ROOT_DIR = process.env.ROOT_DIR || process.cwd()

export async function LinePlot({
  height,
  width,
  yaxis,
  file,
  yaxis_pretty_string,
  caption
}: {
  height: number;
  width: number;
  yaxis: string;
  file: string;
  yaxis_pretty_string: string;
  caption: string;
}) {
  const connection = await DuckDBConnection.create();

  function template_query(file: string) {
    return `create or replace temp table results as
        select distinct on(name)
          ${yaxis},
          substr(name, position('_' in name)+1,
            position('/' in name)-position('_' in name)-1) as algorithm,
          substr(name,position('/' in name)+1,
            length(name)-position('/' in name))::int as input_size
        from '${ROOT_DIR}/${file}'
        where iterations is not null
           and input_size < (1 << 10)
        `
  }
  const query = template_query(file)

  await connection.run(query)

  let reader = await connection.runAndReadAll(`select max(input_size) max_input_size,
    max(${yaxis}) max_yaxis from results`)
  let { max_input_size, max_yaxis } = reader.getRowObjects()[0];

  async function getPoints(alg, connection, name) {
    const reader = await connection.runAndReadAll(
      `select ${yaxis},input_size from results
      where algorithm = '${alg}'
      order by input_size asc`);
    const rows = reader.getRowObjects();
    const points = rows.map((obj) => {return {x: obj.input_size,y: obj[yaxis]}})

    return {points, className: alg, name: name}
  }

  let lines: any[] = []
  lines.push(await getPoints('BinarySearchRandomTarget',connection,'Binary Search'))
  lines.push(await getPoints('LinearSearch',connection,'Linear Search'))

  return (
    <div className={'plotWrapper'}>
    <LinePlotClient height={height} width={width} maxx={max_input_size?.valueOf()}
      maxy={max_yaxis?.valueOf()}
      lines={lines}
      metadata={{xName: 'Number of Elements', yName: yaxis_pretty_string, caption}}
    />
    </div>
  )
}