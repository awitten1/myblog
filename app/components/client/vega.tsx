
'use client'

import React, { useEffect, useRef } from 'react'
import * as vega from 'vega'

type VegaChartProps = {
  spec: vega.Spec
  className?: string
  renderer?: 'canvas' | 'svg'
  hover?: boolean
}

export function VegaChart({
  spec,
  className,
  renderer = 'canvas',
  hover = true,
}: VegaChartProps) {
  const containerRef = useRef<HTMLDivElement>(null)

  useEffect(() => {
    if (!containerRef.current) return

    const view = new vega.View(vega.parse(spec), {
      renderer,
      container: containerRef.current,
      hover,
    })

    view.run()

    return () => {
      view.finalize()
    }
  }, [spec, renderer, hover])

  return <div className={className} ref={containerRef} />
}
