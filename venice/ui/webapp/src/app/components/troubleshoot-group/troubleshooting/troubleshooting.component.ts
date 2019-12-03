import { Component, ViewEncapsulation, OnInit, ViewChild } from '@angular/core';
import * as d3 from 'd3';
import { Node, Link, NodeType, NodeStates, LinkStates } from '@app/models/frontend/shared/networkgraph.interface';
import { Utility } from '@app/common/Utility';
import { NodeConsts } from './networkgraph/networkgraph.component';
import { FormControl } from '@angular/forms';
import { NetworkgraphComponent } from './networkgraph/networkgraph.component';
import { ControllerService } from '@app/services/controller.service';

@Component({
  selector: 'app-troubleshooting',
  templateUrl: './troubleshooting.component.html',
  styleUrls: ['./troubleshooting.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class TroubleshootingComponent implements OnInit {
  @ViewChild('networkgraph') graph: NetworkgraphComponent;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/icon-troubleshooting.svg'
  };
  toolOptions: string[] = [ 'Ping', 'Traceroute', 'Span'];
  graphBufferSide = 120;
  graphBufferBottom = 40;
  graphBufferTop = 40;

  runForceLayout = false;
  nodes: Node[] = [];
  links: Link[] = [];
  showTabs = false;
  sourceFormControl: FormControl = new FormControl('', []);
  destFormControl: FormControl = new FormControl('', []);
  sourceSubmitted: boolean = false;
  destSubmitted: boolean = false;
  sourceNodeID: string = 's1';
  destNodeID: string = 'd1';
  sourceIP: string;
  destIP: string;
  tabsTriggered: boolean = false;

  constructor(protected controllerService: ControllerService) { }

  ngOnInit() {
    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Troubleshooting (mocked data)', url: Utility.getBaseUIUrl() + 'troubleshoot/troubleshooting' }]
    });
    this.defaultLayout();
  }

  tabChange(event) {
    const newTab = event.tab.textLabel;
    switch (newTab) {
      case 'EVENTS':
        this.positionNodesEvents();
        break;
      case 'INGRESS POLICIES':
        this.positionNodesIngressPolicies();
        break;
    }
  }

  submitSource() {
    this.sourceSubmitted = true;
    this.sourceIP = this.sourceFormControl.value;
    this.positionSourceSet();
    if (this.destSubmitted && !this.tabsTriggered) {
      this.tabsTriggered = true;
      this.positionLoadingTabs();
      setTimeout(() => {
        this.showTabs = true;
        this.positionNodesEvents();
      }, 3000);
    }
  }

  submitDest() {
    this.destSubmitted = true;
    this.destIP = this.destFormControl.value;
    this.positionDestSet();
    if (this.sourceSubmitted && !this.tabsTriggered) {
      this.tabsTriggered = true;
      this.positionLoadingTabs();
      setTimeout(() => {
        this.showTabs = true;
        this.positionNodesEvents();
      }, 3000);
    }
  }

  getGraphDimensions() {
    const $ = Utility.getJQuery();
    // TODO: Use height of div
    // currently its taking height before flex applies
    // const height = $("#troubleshooting-graph").height();
    const height = 415;
    const width = $('#troubleshooting-graph').width();
    return {height: height, width: width};
  }

  defaultLayout() {
    const dim = this.getGraphDimensions();
    const height = dim.height;
    const width = dim.width;
    const buffer = this.graphBufferSide + NodeConsts[NodeType.workloadSource].radius;
    this.nodes = [
      {x: buffer, y: height / 2, label: '', secondaryLabel: '', type: NodeType.workloadSource, state: NodeStates.healthy, id: this.sourceNodeID},
      {x: width - buffer, y: height / 2, label: '', secondaryLabel: '', type: NodeType.workloadDestination, state: NodeStates.unknown, id: this.destNodeID},
    ];
    this.links = [];
    this.graph.drawGraph(this.nodes, this.links);
  }

  positionLoadingTabs() {
    const sourceNode = this.nodes.filter((n) => {
      return n.id === this.sourceNodeID;
    })[0];
    sourceNode.state = NodeStates.loading;
    const targetNode = this.nodes.filter((n) => {
      return n.id === this.destNodeID;
    })[0];
    targetNode.state = NodeStates.loading;
    this.graph.drawGraph(this.nodes, this.links);
  }

  /* Expects graph to be in defaultLayout setup
   * only updates the source label and the link.
   * This is so that it doesn't matter if the dest node is set first
   */
  positionSourceSet() {
    const dim = this.getGraphDimensions();
    const height = dim.height;
    const width = dim.width;
    const sourceNode = this.nodes.filter((n) => {
      return n.id === this.sourceNodeID;
    })[0];
    sourceNode.label = 'workload-12345';
    sourceNode.secondaryLabel = this.sourceIP;
    sourceNode.state = NodeStates.pulsing;
    const links = [
      {sourceID: this.sourceNodeID, targetID: this.destNodeID, directed: false, state: LinkStates.neutral},
    ];
    this.links = links;
    this.graph.drawGraph(this.nodes, this.links);
  }

  /* Expects graph to be in defaultLayout setup
   * only updates the source label and the link.
   * This is so that it doesn't matter if the dest node is set first
   */
  positionDestSet() {
    const dim = this.getGraphDimensions();
    const height = dim.height;
    const width = dim.width;
    const destNode = this.nodes.filter((n) => {
      return n.id === this.destNodeID;
    })[0];
    destNode.label = 'workload-34567';
    destNode.secondaryLabel = this.destIP;
    destNode.state = NodeStates.healthy;
    // If there any links, then we must have set src node already.
    // don't redraw link
    if (this.links.length === 1) {
      this.links[0].animate = false;
    }
    this.graph.drawGraph(this.nodes, this.links);
  }

  positionNodesEvents() {
    // x and y positioins are irrelevant, will be reset
    const nodes = [
      {x: 50, y: 300, label: 'Workload-12345', secondaryLabel: this.sourceIP, type: NodeType.workloadSource, state: NodeStates.healthy, id: this.sourceNodeID},
      {x: 50, y: 300, label: 'Naples 1', type: NodeType.naples, state: NodeStates.healthy, id: 'n2'},
      {x: 350, y: 300, type: NodeType.network, state: NodeStates.healthy, id: 'n3'},
      {x: 350, y: 300, label: 'Naples 2', type: NodeType.naples, state: NodeStates.healthy, id: 'n4'},
      {x: 650, y: 300, label: 'Workload-34567', secondaryLabel: this.destIP, type: NodeType.workloadDestination, state: NodeStates.healthy, id: this.destNodeID},
    ];
    const links = [
      {sourceID: this.sourceNodeID, targetID: 'n2', directed: false, state: LinkStates.healthy},
      {sourceID: 'n2', targetID: 'n3', directed: false, state: LinkStates.healthy},
      {sourceID: 'n3', targetID: 'n4', directed: false, state: LinkStates.healthy},
      {sourceID: 'n4', targetID: this.destNodeID, directed: false, state: LinkStates.healthy},
    ];
    let nodeRadiusSum = 0;
    nodes.forEach((n: any) => {
      nodeRadiusSum += 2 * NodeConsts[n.type].radius;
    });

    const dim = this.getGraphDimensions();
    const height = dim.height;
    const width = dim.width;

    // Length of links will be the same between all the nodes
    const linkLength = (width - 2 * this.graphBufferSide - nodeRadiusSum) / (nodes.length - 1);

    let startX = this.graphBufferSide;
    nodes.forEach((n: any) => {
      startX += NodeConsts[n.type].radius;
      n.x = startX;
      n.y = height / 2;
      startX += linkLength + NodeConsts[n.type].radius;
    });

    this.nodes = nodes;
    this.links = links;
    this.graph.drawGraph(this.nodes, this.links);
  }

  positionNodesIngressPolicies() {
    const dim = this.getGraphDimensions();
    const height = dim.height;
    const width = dim.width;
    const buffer = this.graphBufferSide + NodeConsts[NodeType.workloadSource].radius;
    const nodes: Node[] = [
      {x: buffer, y: height / 2, label: 'Workload-12345', secondaryLabel: this.sourceIP, type: NodeType.workloadSource, state: NodeStates.healthy, id: this.sourceNodeID},
      {x: width - buffer, y: height / 2, label: 'Workload-34567', secondaryLabel: this.destIP, type: NodeType.workloadDestination, state: NodeStates.healthy, id: this.destNodeID},
    ];
    const sg1X = buffer + NodeConsts[NodeType.workloadSource].radius + 150 + NodeConsts[NodeType.securityGroup].radius;
    const sg2X = width - buffer  - NodeConsts[NodeType.workloadSource].radius - 150 - NodeConsts[NodeType.securityGroup].radius;
    const spX = sg1X + (sg2X - sg1X - 2 * NodeConsts[NodeType.securityGroup].radius) / 2;

    // adding security groups
    const sgCount = 4;
    const sgRadiusTotal = sgCount * 2 * NodeConsts[NodeType.securityGroup].radius;
    const nodeSpace = (height - this.graphBufferBottom - this.graphBufferTop - sgRadiusTotal) / (sgCount - 1);

    let startY = this.graphBufferTop;
    for (let i = 0; i < sgCount; i++) {
      startY += NodeConsts[NodeType.securityGroup].radius;
      const sgNode1: Node = {x: sg1X, y: startY, label: 'Security Group ' + i, type: NodeType.securityGroup, state: NodeStates.healthy, id: 'sg' + i};
      const sgNode2: Node = {x: sg2X, y: startY, label: 'Security Group ' + i, type: NodeType.securityGroup, state: NodeStates.healthy, id: 'sg' + i + '-1'};
      nodes.push(sgNode1);
      nodes.push(sgNode2);
      startY += nodeSpace + NodeConsts[NodeType.securityGroup].radius;
    }

    // adding security policies
    const spCount = 4;
    const spRadiusTotal = spCount * 2 * NodeConsts[NodeType.securityPolicy].radius;
    const nodeSpaceSp = (height - this.graphBufferBottom - this.graphBufferTop - spRadiusTotal) / (spCount - 1);

    startY = this.graphBufferTop;
    for (let i = 0; i < sgCount; i++) {
      startY += NodeConsts[NodeType.securityPolicy].radius;
      const spNode = {x: spX, y: startY, label: 'Security Policy ' + i, type: NodeType.securityPolicy, state: NodeStates.healthy, id: 'sp' + i};
      if (i === 1) {
        spNode.state = NodeStates.pulsing;
      }
      nodes.push(spNode);
      startY += nodeSpaceSp + NodeConsts[NodeType.securityPolicy].radius;
    }


    const links = [
      // Source to SG group 1
      {sourceID: this.sourceNodeID, targetID: 'sg0', directed: true, state: LinkStates.healthy},
      {sourceID: this.sourceNodeID, targetID: 'sg1', directed: true, state: LinkStates.healthy},
      {sourceID: this.sourceNodeID, targetID: 'sg2', directed: true, state: LinkStates.healthy},
      {sourceID: this.sourceNodeID, targetID: 'sg3', directed: true, state: LinkStates.healthy},
      // SG group 2 to Dest
      {sourceID: 'sg0-1', targetID: this.destNodeID, directed: true, state: LinkStates.healthy},
      {sourceID: 'sg1-1', targetID: this.destNodeID, directed: true, state: LinkStates.healthy},
      {sourceID: 'sg2-1', targetID: this.destNodeID, directed: true, state: LinkStates.healthy},
      {sourceID: 'sg3-1', targetID: this.destNodeID, directed: true, state: LinkStates.healthy},

      // SG to Policy
      {sourceID: 'sg0', targetID: 'sp0', directed: true, state: LinkStates.healthy},
      {sourceID: 'sg1', targetID: 'sp1', directed: true, state: LinkStates.healthy},
      {sourceID: 'sg2', targetID: 'sp2', directed: true, state: LinkStates.healthy},
      {sourceID: 'sg3', targetID: 'sp3', directed: true, state: LinkStates.healthy},

      // Policy to SG
      {sourceID: 'sp0', targetID: 'sg0-1', directed: true, state: LinkStates.healthy},
      {sourceID: 'sp1', targetID: 'sg1-1', directed: true, state: LinkStates.error},
      {sourceID: 'sp2', targetID: 'sg2-1', directed: true, state: LinkStates.healthy},
      {sourceID: 'sp3', targetID: 'sg3-1', directed: true, state: LinkStates.healthy},
    ];


    this.nodes = nodes;
    this.links = links;
    this.graph.drawGraph(this.nodes, this.links);

  }
}
