'use client'

import React, { useEffect, useRef, useState } from 'react'
import * as vega from 'vega'
import './vega.css'

type VegaChartProps = {
  spec: vega.Spec
  className?: string
  renderer?: 'canvas' | 'svg'
  hover?: boolean
}

type ThemeColors = {
  background: string
  text: string
  line: string
  grid: string
  primary: string
  hover: string
}

function readThemeColors(): ThemeColors {
  const styles = getComputedStyle(document.documentElement)
  const background = styles.getPropertyValue('--vega-background').trim() || '#ffffff'
  const text = styles.getPropertyValue('--vega-text').trim() || '#111111'
  const line = styles.getPropertyValue('--vega-line').trim() || '#9aa0a6'
  const grid = styles.getPropertyValue('--vega-grid').trim() || '#e0e0e0'
  const primary = styles.getPropertyValue('--vega-primary').trim() || '#2e3f55'
  const hover = styles.getPropertyValue('--vega-hover').trim() || '#ef4444'

  return {
    background,
    text,
    line,
    grid,
    primary,
    hover,
  }
}

function cloneSpec(spec: vega.Spec): vega.Spec {
  if (typeof structuredClone === 'function') {
    return structuredClone(spec)
  }
  return JSON.parse(JSON.stringify(spec)) as vega.Spec
}

function applyThemeToSpec(spec: vega.Spec, theme: ThemeColors): vega.Spec {
  const themedSpec = cloneSpec(spec)

  themedSpec.background = theme.background
  themedSpec.config = {
    ...themedSpec.config,
    axis: {
      domainColor: theme.line,
      tickColor: theme.line,
      labelColor: theme.text,
      titleColor: theme.text,
      gridColor: theme.grid,
      ...themedSpec.config?.axis,
    },
    legend: {
      labelColor: theme.text,
      titleColor: theme.text,
      ...themedSpec.config?.legend,
    },
    title: {
      color: theme.text,
      ...themedSpec.config?.title,
    },
    text: {
      color: theme.text,
      ...themedSpec.config?.text,
    },
    range: {
      category: [theme.primary, theme.line],
      ...themedSpec.config?.range,
    },
    mark: {
      color: theme.primary,
      ...themedSpec.config?.mark,
    },
  }

  const marks = (themedSpec as { marks?: Array<Record<string, unknown>> }).marks
  if (Array.isArray(marks)) {
    marks.forEach((mark) => {
      const encode = mark.encode as Record<string, Record<string, unknown>> | undefined
      const enter = encode?.enter as Record<string, unknown> | undefined
      const update = encode?.update as Record<string, unknown> | undefined
      const hover = encode?.hover as Record<string, unknown> | undefined

      const enterFill = enter?.fill as { value?: unknown } | undefined
      if (enterFill && typeof enterFill.value === 'string') {
        enterFill.value = theme.primary
      }

      const updateFill = update?.fill as { value?: unknown } | undefined
      if (updateFill && typeof updateFill.value === 'string') {
        updateFill.value = theme.primary
      }

      const hoverFill = hover?.fill as { value?: unknown } | undefined
      if (hoverFill && typeof hoverFill.value === 'string') {
        hoverFill.value = theme.hover
      }

      const enterTextFill = enter?.fill as { value?: unknown } | undefined
      if (mark.type === 'text' && enterTextFill && typeof enterTextFill.value === 'string') {
        enterTextFill.value = theme.text
      }
    })
  }

  return themedSpec
}

export function VegaChart({
  spec,
  className,
  renderer = 'canvas',
  hover = true,
}: VegaChartProps) {
  const containerRef = useRef<HTMLDivElement>(null)
  const [themeVersion, setThemeVersion] = useState(0)

  useEffect(() => {
    const root = document.documentElement
    const observer = new MutationObserver(() => {
      setThemeVersion((current) => current + 1)
    })

    observer.observe(root, { attributes: true, attributeFilter: ['data-theme'] })

    return () => observer.disconnect()
  }, [])

  useEffect(() => {
    if (!containerRef.current) return

    const theme = readThemeColors()
    const themedSpec = applyThemeToSpec(spec, theme)
    const view = new vega.View(vega.parse(themedSpec), {
      renderer,
      container: containerRef.current,
      hover,
    })

    view.run()

    return () => {
      view.finalize()
    }
  }, [spec, renderer, hover, themeVersion])

  return <div className={className} ref={containerRef} />
}
