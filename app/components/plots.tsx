import React from 'react'
import { readFile } from "fs/promises"
import { LinePlotClient, Point, Line } from "./client/graph"
import { DuckDBConnection } from '@duckdb/node-api';
import * as d3 from "d3";
import './plot.css'

/**
 select name, (macroops_retired) / (macroops_dispatched) as fraction_of_uops_that_do_not_retire
    from (
      select
        name,
        "perf_raw::r4307AA" as macroops_dispatched,
        "perf_raw::r4300C1" macroops_retired,
        "perf_raw::r430076"*8 as dispatch_slots
      from 'results/amdzen5/bad_speculation.csv'
    )
 */

/**

create temp table bad_speculation as
  select name,
    "perf_raw::r4307AA" as macroops_dispatched,
    "perf_raw::r4300C1" macroops_retired,
    "perf_raw::r430076"*8 as dispatch_slots
  from 'results/amdzen5/bad_speculation.csv';

create temp table frontend_backend_speculation as
  select name,
    "perf_raw::r100431EA0" as unused_dispatch_slots_backend,
    "perf_raw::r1004301A0" as unused_dispatch_slots_frontend,
    "perf_raw::r430076"*8 as dispatch_slots
  from 'results/amdzen5/frontend_backend_bound.csv';

from frontend_backend_speculation f join bad_speculation b using (name)
  select f.name,
    f.dispatch_slots,
    b.macroops_dispatched,
    f.unused_dispatch_slots_backend,
    f.unused_dispatch_slots_frontend
  order by f.dispatch_slots desc;

  It should be true that:
  macro-ops dispatched + unused dispatch slots = dispatch_slots

 */

const ROOT_DIR = process.env.ROOT_DIR || process.cwd()

export async function LinePlot({
  height,
  width,
  yaxis,
  yaxis_expr,
  yaxis_key,
  file,
  series,
  yaxis_pretty_string,
  caption
}: {
  height: number;
  width: number;
  yaxis: string;
  yaxis_expr?: string;
  yaxis_key?: string;
  file: string;
  series?: {
    name: string;
    className?: string;
    algorithm?: string;
    where?: string;
  }[];
  yaxis_pretty_string: string;
  caption: string;
}) {
  const connection = await DuckDBConnection.create();

  const yaxisExpression = yaxis_expr || yaxis;
  const yaxisAlias = (yaxis_key || yaxis)
    .replaceAll('-', '_')
    .replaceAll('"', '');

  function template_query(file: string) {
    return `create or replace temp table results as
        select distinct on(name)
          ${yaxisExpression} as ${yaxisAlias},
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

  function getSeriesFilter(seriesItem: {
    name: string;
    className?: string;
    algorithm?: string;
    where?: string;
  }) {
    if (seriesItem.where) {
      return seriesItem.where
    }

    if (seriesItem.algorithm) {
      return `algorithm = '${seriesItem.algorithm}'`
    }

    throw new Error('LinePlot series requires an algorithm or where clause.')
  }

  const seriesList = series ?? [
    {
      name: 'Binary Search',
      className: 'BinarySearchRandomTarget',
      algorithm: 'BinarySearchRandomTarget',
    },
    {
      name: 'Linear Search',
      className: 'LinearSearch',
      algorithm: 'LinearSearch',
    },
  ]

  const seriesFilters = seriesList.map((seriesItem) => getSeriesFilter(seriesItem))
  const seriesWhere = seriesFilters.map((filter) => `(${filter})`).join(' OR ')

  async function getPoints(filter: string, name: string, className: string) {
    const reader = await connection.runAndReadAll(
      `select ${yaxisAlias},input_size from results
      where ${filter}
      order by input_size asc`);
    const rows = reader.getRowObjects();
    const points = rows
      .map((obj) => ({
        x: Number(obj.input_size),
        y: Number(obj[yaxisAlias]),
      }))
      .filter((point) => Number.isFinite(point.x) && Number.isFinite(point.y))

    return { points, className, name }
  }

  let reader = await connection.runAndReadAll(`select max(input_size) max_input_size,
    max(${yaxisAlias}) max_yaxis from results
    where ${seriesWhere}`)
  let { max_input_size, max_yaxis } = reader.getRowObjects()[0];

  let lines: Line[] = []
  for (let seriesItem of seriesList) {
    const filter = getSeriesFilter(seriesItem)
    const className = seriesItem.className || seriesItem.algorithm || seriesItem.name
    lines.push(await getPoints(filter, seriesItem.name, className))
  }

  return (
    <div className={'plotWrapper'}>
      <LinePlotClient height={height} width={width}
        maxx={Number(max_input_size) || 0}
        maxy={Number(max_yaxis) || 0}
        lines={lines}
        metadata={{ xName: 'Number of Elements', yName: yaxis_pretty_string, caption }}
      />
    </div>
  )
}
