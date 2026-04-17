'use client'

import React, { useEffect, useRef, useState } from 'react'
import * as d3 from 'd3'
import './graph_styles.css'

export interface FacetSeries {
  name: string
  points: { x: number; y: number }[]
}

export interface Facet {
  label: string
  series: FacetSeries[]
}

interface Props {
  facets: Facet[]
  xName: string
  yName: string
  caption: string
  facetWidth?: number
  facetHeight?: number
}

const PALETTE: Record<string, string> = {
  random: '#e45756',
  strided: '#4c78a8',
  sa_rv: '#54a24b',
}

function color(name: string): string {
  return PALETTE[name] ?? '#888'
}

interface HoverPoint {
  seriesName: string
  x: number
  y: number
  cx: number
  cy: number
}

function FacetChart({
  facet,
  xDomain,
  yDomain,
  width,
  height,
  xName,
  yName,
  showY,
  clipId,
  registerReset,
}: {
  facet: Facet
  xDomain: [number, number]
  yDomain: [number, number]
  width: number
  height: number
  xName: string
  yName: string
  showY: boolean
  clipId: string
  registerReset: (fn: () => void) => void
}) {
  const svgRef = useRef<SVGSVGElement>(null)
  const wrapRef = useRef<HTMLDivElement>(null)
  const [hover, setHover] = useState<HoverPoint | null>(null)

  const margin = { top: 34, right: 24, bottom: 52, left: showY ? 80 : 24 }
  const innerW = width - margin.left - margin.right
  const innerH = height - margin.top - margin.bottom

  // Current zoom transform is mirrored into a ref so the mousemove handler
  // (which doesn't re-run the d3 effect) can map pointer coords through it.
  const transformRef = useRef<d3.ZoomTransform>(d3.zoomIdentity)

  useEffect(() => {
    if (!svgRef.current) return
    const svg = d3.select(svgRef.current)
    svg.selectAll('*').remove()

    const x = d3.scaleLinear().domain(xDomain).range([0, innerW]).nice()
    const y = d3.scaleLinear().domain(yDomain).range([innerH, 0]).nice()

    // clip path so zoomed-in data doesn't escape the plot area
    svg
      .append('defs')
      .append('clipPath')
      .attr('id', clipId)
      .append('rect')
      .attr('x', 0)
      .attr('y', 0)
      .attr('width', innerW)
      .attr('height', innerH)

    const root = svg
      .append('g')
      .attr('transform', `translate(${margin.left},${margin.top})`)

    // capture pointer events across the whole plot area so panning works
    // even when the cursor is over empty space
    root
      .append('rect')
      .attr('width', innerW)
      .attr('height', innerH)
      .style('fill', 'transparent')
      .style('pointer-events', 'all')

    const xAxisG = root
      .append('g')
      .attr('class', 'lap-axis')
      .attr('transform', `translate(0,${innerH})`)

    const yAxisG = root.append('g').attr('class', 'lap-axis')

    const gridG = root.append('g').attr('class', 'lap-grid')

    const plotG = root.append('g').attr('clip-path', `url(#${clipId})`)

    const seriesGroups = facet.series.map((s) => {
      const c = color(s.name)
      const g = plotG.append('g')
      const path = g
        .append('path')
        .datum(s.points)
        .style('fill', 'none')
        .style('stroke', c)
        .style('stroke-width', '2px')
      const circles = g
        .append('g')
        .selectAll('circle')
        .data(s.points)
        .join('circle')
        .attr('r', 2.2)
        .style('fill', c)
        .style('stroke', c)
      return { path, circles }
    })

    root
      .append('text')
      .attr('x', innerW / 2)
      .attr('y', -12)
      .attr('text-anchor', 'middle')
      .attr('class', 'lap-facet-title')
      .text(facet.label)

    root
      .append('text')
      .attr('x', innerW / 2)
      .attr('y', innerH + 42)
      .attr('text-anchor', 'middle')
      .attr('class', 'axis-label')
      .text(xName)

    if (showY) {
      root
        .append('text')
        .attr('transform', `translate(${-58},${innerH / 2}) rotate(-90)`)
        .attr('text-anchor', 'middle')
        .attr('class', 'axis-label')
        .text(yName)
    }

    function redraw(t: d3.ZoomTransform) {
      const zx = t.rescaleX(x)
      const zy = t.rescaleY(y)

      xAxisG.call(d3.axisBottom(zx).ticks(6).tickSizeOuter(0))
      if (showY) {
        yAxisG.call(d3.axisLeft(zy).ticks(6).tickSizeOuter(0))
      }

      gridG
        .selectAll<SVGLineElement, number>('line')
        .data(zy.ticks(6))
        .join('line')
        .attr('x1', 0)
        .attr('x2', innerW)
        .attr('y1', (d) => zy(d))
        .attr('y2', (d) => zy(d))

      const line = d3
        .line<{ x: number; y: number }>()
        .x((d) => zx(d.x))
        .y((d) => zy(d.y))
        .curve(d3.curveMonotoneX)

      seriesGroups.forEach(({ path, circles }) => {
        path.attr('d', line as any)
        circles.attr('cx', (d) => zx(d.x)).attr('cy', (d) => zy(d.y))
      })
    }

    const zoom = d3
      .zoom<SVGSVGElement, unknown>()
      .scaleExtent([1, 40])
      .translateExtent([
        [-margin.left, -margin.top],
        [innerW + margin.right, innerH + margin.bottom],
      ])
      .extent([
        [0, 0],
        [innerW, innerH],
      ])
      .on('zoom', (event) => {
        transformRef.current = event.transform
        redraw(event.transform)
      })

    svg.call(zoom)
    svg.on('dblclick.zoom', null)

    registerReset(() => {
      svg.transition().duration(250).call(zoom.transform, d3.zoomIdentity)
    })

    redraw(d3.zoomIdentity)
  }, [facet, xDomain, yDomain, innerW, innerH, margin.left, margin.top, margin.right, margin.bottom, showY, xName, yName, clipId, registerReset])

  function onMove(event: React.MouseEvent<SVGSVGElement>) {
    if (!svgRef.current) return
    const svgRect = svgRef.current.getBoundingClientRect()
    const scaleX = width / svgRect.width
    const scaleY = height / svgRect.height
    const px = (event.clientX - svgRect.left) * scaleX - margin.left
    const py = (event.clientY - svgRect.top) * scaleY - margin.top

    const x = d3.scaleLinear().domain(xDomain).range([0, innerW]).nice()
    const y = d3.scaleLinear().domain(yDomain).range([innerH, 0]).nice()
    const zx = transformRef.current.rescaleX(x)
    const zy = transformRef.current.rescaleY(y)

    let best: HoverPoint | null = null
    let bestDist = Infinity
    for (const s of facet.series) {
      for (const p of s.points) {
        const cx = zx(p.x)
        const cy = zy(p.y)
        if (cx < 0 || cx > innerW || cy < 0 || cy > innerH) continue
        const d = Math.hypot(px - cx, py - cy)
        if (d < bestDist && d < 40) {
          bestDist = d
          best = { seriesName: s.name, x: p.x, y: p.y, cx, cy }
        }
      }
    }
    setHover(best)
  }

  return (
    <div ref={wrapRef} style={{ position: 'relative', width: '100%' }}>
      <svg
        ref={svgRef}
        width={width}
        height={height}
        viewBox={`0 0 ${width} ${height}`}
        style={{
          maxWidth: '100%',
          height: 'auto',
          display: 'block',
          cursor: 'grab',
        }}
        onMouseMove={onMove}
        onMouseLeave={() => setHover(null)}
      />
      {hover && (
        <>
          <svg
            width={width}
            height={height}
            viewBox={`0 0 ${width} ${height}`}
            style={{
              position: 'absolute',
              inset: 0,
              width: '100%',
              height: '100%',
              pointerEvents: 'none',
            }}
          >
            <circle
              cx={hover.cx + margin.left}
              cy={hover.cy + margin.top}
              r={5}
              style={{
                fill: 'none',
                stroke: color(hover.seriesName),
                strokeWidth: 2,
              }}
            />
          </svg>
          <div
            className="popup"
            style={{
              position: 'absolute',
              left: `calc(${((hover.cx + margin.left) / width) * 100}% + 12px)`,
              top: `calc(${((hover.cy + margin.top) / height) * 100}% + 12px)`,
              pointerEvents: 'none',
              zIndex: 10,
            }}
          >
            <div className="popup-label">
              <strong style={{ color: color(hover.seriesName) }}>
                {hover.seriesName}
              </strong>
              {' · '}
              {facet.label}
            </div>
            <div className="popup-label">
              {xName}: <strong>{hover.x}</strong>
            </div>
            <div className="popup-label">
              {yName}: <strong>{hover.y.toLocaleString()}</strong>
            </div>
          </div>
        </>
      )}
    </div>
  )
}

export function LapPlotClient({
  facets,
  xName,
  yName,
  caption,
  facetWidth = 640,
  facetHeight = 440,
}: Props) {
  const allPoints = facets.flatMap((f) => f.series.flatMap((s) => s.points))
  const xDomain: [number, number] = [
    0,
    (d3.max(allPoints, (p) => p.x) ?? 1) * 1.02,
  ]
  const yDomain: [number, number] = [
    0,
    (d3.max(allPoints, (p) => p.y) ?? 1) * 1.05,
  ]

  const seriesNames = Array.from(
    new Set(facets.flatMap((f) => f.series.map((s) => s.name)))
  )

  const resetFnsRef = useRef<Array<() => void>>([])

  function resetAll() {
    for (const fn of resetFnsRef.current) fn?.()
  }

  return (
    <figure className="plot">
      <div className="legend-container">
        {seriesNames.map((name) => (
          <div key={name} className="legend-item">
            <span
              className="legend-swatch"
              style={{ background: color(name), borderColor: color(name) }}
            />
            <span className="legend-label">{name}</span>
          </div>
        ))}
        <button
          type="button"
          onClick={resetAll}
          className="lap-reset-button"
        >
          Reset zoom
        </button>
      </div>
      <div
        style={{
          display: 'flex',
          flexWrap: 'nowrap',
          justifyContent: 'center',
          alignItems: 'flex-start',
          gap: '1.25rem',
          width: '100%',
        }}
      >
        {facets.map((facet, i) => (
          <div
            key={facet.label}
            style={{ flex: '1 1 0', minWidth: 0, maxWidth: facetWidth }}
          >
            <FacetChart
              facet={facet}
              xDomain={xDomain}
              yDomain={yDomain}
              width={facetWidth}
              height={facetHeight}
              xName={xName}
              yName={yName}
              showY={i === 0}
              clipId={`lap-clip-${i}`}
              registerReset={(fn) => {
                resetFnsRef.current[i] = fn
              }}
            />
          </div>
        ))}
      </div>
      <figcaption>{caption}</figcaption>
    </figure>
  )
}
