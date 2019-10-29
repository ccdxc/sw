import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { Link, LinkStates, MarkerTypes, Node, NodeStates } from '@app/models/frontend/shared/networkgraph.interface';
import * as d3 from 'd3';
import { DefaultLinkObject } from 'd3';

export const NodeConsts = {
  workloadSource: {
    class: 'networkgraph-node-workloadSource',
    labelClass: 'networkgraph-node-workloadSource-label',
    secondaryLabelClass: 'networkgraph-node-workloadSource-label-secondary',
    radius: 35,
    labelOffset: 30,
    image: {
      loading: {
        url: '/assets/images/icons/networkgraph/ico-workload-source.svg',
        size: 45
      },
      unknown: {
        url: '',
        size: 45
      },
      pulsing: {
        url: '/assets/images/icons/networkgraph/ico-workload-source.svg',
        size: 45
      },
      error: {
        url: '',
        size: 45
      },
      healthy: {
        url: '/assets/images/icons/networkgraph/ico-workload-source.svg',
        size: 45
      },
    }
  },
  workloadDestination: {
    class: 'networkgraph-node-workloadDestination',
    labelClass: 'networkgraph-node-workloadDestination-label',
    secondaryLabelClass: 'networkgraph-node-workloadDestination-label-secondary',
    radius: 35,
    labelOffset: 30,
    image: {
      loading: {
        url: '/assets/images/icons/networkgraph/workloadDestination/ico-workload-destination-healthy.svg',
        size: 45
      },
      unknown: {
        url: '/assets/images/icons/networkgraph/workloadDestination/ico-workload-destination-unknown.svg',
        size: 45
      },
      pulsing: {
        url: '',
        size: 45
      },
      error: {
        url: '',
        size: 45
      },
      healthy: {
        url: '/assets/images/icons/networkgraph/workloadDestination/ico-workload-destination-healthy.svg',
        size: 45
      },
    }
  },
  securityGroup: {
    class: 'networkgraph-node-securityGroup',
    labelClass: 'networkgraph-node-securityGroup-label',
    secondaryLabelClass: 'networkgraph-node-securityGroup-label-secondary',
    radius: 20,
    labelOffset: 25,
    image: {
      loading: {
        url: '',
        size: 30
      },
      unknown: {
        url: '',
        size: 30
      },
      pulsing: {
        url: '',
        size: 30
      },
      error: {
        url: '',
        size: 30
      },
      healthy: {
        url: '/assets/images/icons/networkgraph/ico-security-group.svg',
        size: 30
      },
    }
  },
  securityPolicy: {
    class: 'networkgraph-node-securityPolicy',
    labelClass: 'networkgraph-node-securityPolicy-label',
    secondaryLabelClass: 'networkgraph-node-securityPolicy-label-secondary',
    radius: 15,
    labelOffset: 25,
    image: {
      loading: {
        url: '',
        size: 19
      },
      unknown: {
        url: '',
        size: 19
      },
      pulsing: {
        url: '/assets/images/icons/networkgraph/securityPolicy/ico-security-policy-error.svg',
        size: 19
      },
      error: {
        url: '/assets/images/icons/networkgraph/securityPolicy/ico-security-policy-error.svg',
        size: 19
      },
      healthy: {
        url: '/assets/images/icons/networkgraph/securityPolicy/ico-security-policy-healthy.svg',
        size: 19
      },
    }
  },
  rule: {},
  naples: {
    class: 'networkgraph-node-naples',
    labelClass: 'networkgraph-node-naples-label',
    secondaryLabelClass: 'networkgraph-node-naples-label-secondary',
    radius: 25,
    labelOffset: 25,
    image: {
      loading: {
        url: '',
        size: 24
      },
      unknown: {
        url: '',
        size: 24
      },
      pulsing: {
        url: '',
        size: 24
      },
      error: {
        url: '',
        size: 24
      },
      healthy: {
        url: '/assets/images/icons/networkgraph/ico-naples.svg',
        size: 24
      },
    }
  },
  network: {
    class: 'networkgraph-node-network',
    labelClass: 'networkgraph-node-network-label',
    secondaryLabelClass: 'networkgraph-node-network-label-secondary',
    labelOffset: 15,
    radius: 45,
    image: {
      loading: {
        url: '',
        size: 60
      },
      unknown: {
        url: '',
        size: 60
      },
      pulsing: {
        url: '',
        size: 60
      },
      error: {
        url: '',
        size: 60
      },
      healthy: {
        url: '/assets/images/icons/networkgraph/ico-service-copy.svg',
        size: 60
      },
    }
  }
};

@Component({
  selector: 'app-networkgraph',
  templateUrl: './networkgraph.component.html',
  styleUrls: ['./networkgraph.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class NetworkgraphComponent implements OnInit {
  // TODO: Cache some of the functions for improved performance

  runForceLayout = false;
  nodes: Node[] = [];
  links: Link[] = [];

  nodeStateClasses: any = {
    healthy: '',
    unknown: 'networkgraph-nodestate-unknown',
    pulsing: 'networkgraph-nodestate-pulsing'
  };

  linkStateClasses: any = {
    healthy: 'networkgraph-linkstate-healthy',
    neutral: 'networkgraph-linkstate-neutral',
    error: 'networkgraph-linkstate-error',
  };

  _animate: boolean = true;

  settings: any = {
  };


  constructor() { }

  ngOnInit() {
    this.setupMarkers();
  }

  setupMarkers() {
    // marker
    const svg = d3.select('#networkgraph-graph');
    const arrowMarkers = [
      {
        name: 'arrow-healthy',
        class: 'networkgraph-marker-healthy'
      },
      {
        name: 'arrow-error',
        class: 'networkgraph-marker-error'
      },
    ];
    const circleMarkers = [
      {
        name: 'circle-healthy',
        class: 'networkgraph-marker-healthy'
      },
      {
        name: 'circle-error',
        class: 'networkgraph-marker-error'
      }
    ];

    const defs = svg.append('svg:defs');
    arrowMarkers.forEach((markerSettings) => {
      console.log(markerSettings.name );
          defs.append('svg:marker')    // This section adds in the arrows
          .attr('id', markerSettings.name)
          .attr('viewBox', '0 -5 10 10')
          .attr('refX', 11)
          .attr('refY', 0)
          .attr('markerWidth', 8)
          .attr('markerHeight', 8)
          .attr('markerUnits', 'strokeWidth')
          .attr('orient', 'auto')
        .append('svg:path')
          .attr('d', 'M0,-5L10,0L0,5')
          .attr('class', markerSettings.class);
    });

    circleMarkers.forEach((markerSettings) => {
          defs.append('svg:marker')    // This section adds in the arrows
          .attr('id', markerSettings.name)
          .attr('viewBox', '-6 -6 12 12')
          .attr('refX', 0)
          .attr('refY', 0)
          .attr('markerWidth', 11)
          .attr('markerHeight', 12)
          .attr('markerUnits', 'strokeWidth')
          .attr('orient', 'auto')
        .append('svg:path')
          .attr('d', 'M 0, 0  m -2, 0  a 2,2 0 1,0 4,0  a 2,2 0 1,0 -4,0')
          .attr('class', markerSettings.class);

    });
  }

  drawGraph(nodes, links, runForceLayout = false) {
    this.nodes = nodes;
    this.links = links;
    this.runForceLayout = runForceLayout;
    this.processLinks();

    if (!this.runForceLayout) {
      this.generateStaticLayout();
    }
  }

  processLinks() {
    this.links.forEach( (link: Link) => {
      // set animation
      if (link.animate == null) {
        link.animate = this.animate;
      }
      // set source and target
      if (link.source == null) {
        const s: any = this.getNodeById(link.sourceID);
        link.source = [s.x + NodeConsts[s.type].radius, s.y];
      }
      if (link.target == null) {
        const t: any = this.getNodeById(link.targetID);
        link.target = [t.x - NodeConsts[t.type].radius, t.y];
      }
      // set markers
      if (link.markerStart == null) {
        link.markerStart = this.calculateMarkerStart(link);
      }
      if (link.markerEnd == null) {
        link.markerEnd = this.calculateMarkerEnd(link);
      }
    });
  }

  calculateMarkerStart(link: Link) {
    // Links that are not neutral should have no markers
    // Links that are directed have a circle at source and arrow at target
    // Links that are undirected have a circle at source and at target
    if (link.state === LinkStates.healthy) {
      return MarkerTypes.circleHealthy;
    } else if (link.state === LinkStates.error) {
      return MarkerTypes.circleError;
    } else {
      return MarkerTypes.None;
    }
  }

  calculateMarkerEnd(link: Link) {
    // Links that are not neutral should have no markers
    // Links that are directed have a circle at source and arrow at target
    // Links that are undirected have a circle at source and at target
    if (link.state === LinkStates.neutral) {
      return MarkerTypes.None;
    }
    if (link.directed) {
      if (link.state === LinkStates.healthy) {
        return MarkerTypes.arrowHealthy;
      } else if (link.state === LinkStates.error) {
        return MarkerTypes.arrowError;
      }
    } else {
      if (link.state === LinkStates.healthy) {
        return MarkerTypes.circleHealthy;
      } else if (link.state === LinkStates.error) {
        return MarkerTypes.circleError;
      }
    }
  }

  generateStaticLayout() {
    const svg = d3.select('#networkgraph-graph');
    // TODO: Decide whether this is efficient enough, or if it should compute deltas
    svg.selectAll('g').remove();
    svg.selectAll('.networkgraph-link').remove();

    this.drawNodes();
    this.drawLinks();
  }

  drawNodes() {
    const svg = d3.select('#networkgraph-graph');
    const node = svg.selectAll('g.node').data(this.nodes);
    const nodeEnter = node.enter()
      .append('svg:g')
        .attr('transform', function(d: any) { return 'translate(' + d.x + ',' + d.y + ')'; });

    nodeEnter.append('svg:circle')
      .attr('class', (d: any) => NodeConsts[d.type].class + ' ' + this.nodeStateClasses[d.state])
      .attr('r', (d: any) => NodeConsts[d.type].radius);

    // node text
    nodeEnter.append('text')
      .text((d) => d.label)
      .attr('class', (d: any) => NodeConsts[d.type].labelClass)
      .attr('y', (d: any) => NodeConsts[d.type].radius + NodeConsts[d.type].labelOffset)
      .style('text-anchor', 'middle');

    // node secondary text
    nodeEnter.filter((d) => {return d.secondaryLabel != null;
    }).append('text')
      .text((d) => d.secondaryLabel)
      .attr('class', (d: any) => NodeConsts[d.type].secondaryLabelClass)
      .attr('y', (d: any) => NodeConsts[d.type].radius + (NodeConsts[d.type].labelOffset * 1.75))
      .style('text-anchor', 'middle');

    // Append images
    const images = nodeEnter.append('svg:image')
      .attr('xlink:href',  (d: any) => NodeConsts[d.type].image[d.state].url)
      .attr('x', (d: any) => -(NodeConsts[d.type].image[d.state].size / 2))
      .attr('y', (d: any) => -(NodeConsts[d.type].image[d.state].size / 2))
      .attr('height', (d: any) => NodeConsts[d.type].image[d.state].size)
      .attr('width', (d: any) => NodeConsts[d.type].image[d.state].size);

    // Pulse animation
    nodeEnter.filter((d) => {
        return d.state === NodeStates.pulsing;
      })
      .append('svg:circle')
      .attr('class', (d: any) => NodeConsts[d.type].class + ' ' + this.nodeStateClasses[d.state])
      .style('fill', 'transparent')
      .attr('r', (d: any) => NodeConsts[d.type].radius)
      .transition()
        .duration(600)
        .ease(d3.easeLinear)
        .on('start', function repeat() {
          d3.active(this)
            .attr('r', (d: any) => NodeConsts[d.type].radius * 1.3)
            .transition()
            .duration(400)
              .style('opacity', 0)
            .transition()
            .duration(100)
            .attr('r', (d: any) => NodeConsts[d.type].radius * 1)
            .transition()
            .duration(800)
              .style('opacity', 1)
            .transition()
              .duration(600)
              .on('start', repeat);
        });

    // loading animation
    const gradient = svg.append('svg:defs')
    .append('svg:linearGradient')
    .attr('id', 'gradient')
    .attr('x1', '0%')
    .attr('y1', '0%')
    .attr('x2', '100%')
    .attr('y2', '100%')
    .attr('spreadMethod', 'pad');

    // Define the gradient colors
    gradient.append('svg:stop')
    .attr('offset', '0%')
    .attr('stop-color', '#ffffff')
    .attr('stop-opacity', 1);

    gradient.append('svg:stop')
    .attr('offset', '100%')
    .attr('stop-color', '#a4c7f0')
    .attr('stop-opacity', 1);

    const t0 = Date.now();
    nodeEnter.filter((d) => {
        return d.state === NodeStates.loading;
      })
      .append('svg:circle')
      .attr('class', 'networkgraph-loading')
      .style('fill', 'transparent')
      .style('stroke', 'url(#gradient')
      .attr('r', (d: any) => NodeConsts[d.type].radius * 1.3);

    d3.timer(function () {
      const delta = Date.now() - t0;

      svg.selectAll('.networkgraph-loading')
        .attr('transform', function (d) {
        return 'rotate(' + 90 + delta * 50 / 200 + ')';
      });
    });
      // .transition()
      //   .duration(600)
      //   .ease(d3.easeLinear)
      //   .on('start', function repeat() {
      //     d3.active(this)
      //       .transition()
      //       .duration(400)
      //         .style('opacity', 0)
      //       .transition()
      //       .duration(100)
      //       .attr('r', (d: any) => NodeConsts[d.type].radius * 1)
      //       .transition()
      //       .duration(800)
      //         .style('opacity', 1)
      //       .transition()
      //         .duration(600)
      //         .on('start', repeat);
      //   });

  }

  /**
   * Link generator modeled after d3/d3-shape's linkHorizontal generator
   * This generator allows for the section before and after the bezier curve
   * to be flat, rather than interpolating the curve immediately from the points beginning
   * to the points end.
   */
  linkGenerator() {
    const curveHorizontal = (context, x0, y0, x1, y1) => {
      const origX0 = x0;
      const origX1 = x1;
      // Where to start bezier curve
      x0 += 20;
      // Where to end bezier curve
      x1 -= 20;
      context.moveTo(origX0, y0);
      context.lineTo(x0, y0);
      context.bezierCurveTo(x0 = (x0 + x1) / 2, y0, x0, y1, x1, y1);
      context.lineTo(origX1, y1);
    };
    const link = (linkObj: DefaultLinkObject) => {
      const context = d3.path();
      curveHorizontal(context, linkObj.source[0], linkObj.source[1], linkObj.target[0], linkObj.target[1]);
      return context;
    };
    return link;
  }

  drawLinks() {
    const svg = d3.select('#networkgraph-graph');
    // const linkHorizontal = d3.linkHorizontal()
    const linkGenerator = this.linkGenerator();

    const links = svg.selectAll('.link')
      .data(this.links)
      .enter()
      .append('path')
      .attr('class', (link: Link) => {
        return 'networkgraph-link ' + this.linkStateClasses[link.state];
      })
      .style('fill', 'transparent');

    links.filter((link: Link) => {
        return link.animate;
      })
      // setting source and target as same for animating.
      .attr('d', (link: any): any => {
        const linkObj: DefaultLinkObject = {
          source: link.source,
          target: link.source
        };
        return linkGenerator(linkObj);
      })
      .transition()
      .duration(500)
      .ease(d3.easeLinear)
      .attr('d', (link: Link): any => {
        const linkObj: DefaultLinkObject = {
          source: link.source,
          target: link.target
        };
        return linkGenerator(linkObj);
      });

    // set links that aren't animated
    links.filter( (link) => {
        return !link.animate;
      })
      .attr('d', ( link: Link): any => {
        const linkObj: DefaultLinkObject = {
          source: link.source,
          target: link.target
        };
        return linkGenerator(linkObj);
      });

    links.filter((link: Link) => {
        return link.markerStart !== MarkerTypes.None;
      })
        .attr('marker-start', function(link, i ) {
           return 'url(#' + link.markerStart + ')';
          });

    links.filter((link: any) => {
        return link.markerEnd !== MarkerTypes.None;
      })
      .attr('marker-end', function(link, i ) { return 'url(#' + link.markerEnd + ')'; } );
  }


  getNodeById(id) {
    for (let i = 0; i < this.nodes.length; i++) {
      const currId = this.nodes[i].id;
      if (id === currId) {
        return this.nodes[i];
      }
    }
  }

  get animate(): boolean {
    return this._animate;
  }

  set animate(shouldAnimate: boolean) {
    this._animate = shouldAnimate;
  }
}

