import React from 'react'
import { readFile } from 'fs/promises'
import * as d3 from 'd3'
import { LapPlotClient, Facet } from './client/lap-plot-client'

const ROOT_DIR = process.env.ROOT_DIR || process.cwd()

interface Props {
  file: string
  patterns?: string[]
  coreKinds?: string[]
  xName?: string
  yName?: string
  caption: string
  facetWidth?: number
  facetHeight?: number
}

export async function LapPlot({
  file,
  patterns = ['strided', 'random'],
  coreKinds = ['ecore', 'pcore'],
  xName = 'Iterations',
  yName = 'Cycles (median)',
  caption,
  facetWidth,
  facetHeight,
}: Props) {
  const raw = await readFile(`${ROOT_DIR}/${file}`, 'utf8')
  const firstNewline = raw.indexOf('\n')
  const cleaned = raw.startsWith('pattern,')
    ? raw
    : raw.slice(firstNewline + 1)
  const rows = d3.csvParse(cleaned)

  const facets: Facet[] = coreKinds.map((core) => {
    const series = patterns.map((pattern) => {
      const points = rows
        .filter((r) => r.pattern === pattern && r.core_kind === core)
        .map((r) => ({ x: +r.iters!, y: +r.median! }))
        .sort((a, b) => a.x - b.x)
      return { name: pattern, points }
    })
    return { label: core === 'pcore' ? 'P-core' : 'E-core', series }
  })

  return (
    <div className="plotWrapper">
      <LapPlotClient
        facets={facets}
        xName={xName}
        yName={yName}
        caption={caption}
        facetWidth={facetWidth}
        facetHeight={facetHeight}
      />
    </div>
  )
}
