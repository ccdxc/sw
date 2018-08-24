/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component, DebugElement } from '@angular/core';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { MatIconRegistry } from '@angular/material';

/**-----
 Venice UI -  imports
 ------------------*/
import { ClusterComponent } from './cluster.component';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { ClusterCluster, ClusterNode } from '@sdk/v1/models/generated/cluster';
import { EventsService } from '@app/services/events.service';
import { AlerttableService } from '@app/services/alerttable.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { SearchService } from '@app/services/generated/search.service';

@Component({
  template: ''
})
class DummyComponent { }

describe('ClusterComponent', () => {
  let component: ClusterComponent;
  let fixture: ComponentFixture<ClusterComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        ClusterComponent,
        DummyComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        HttpClientTestingModule,
        NoopAnimationsModule,
        SharedModule,
        MaterialdesignModule,
        PrimengModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        ClusterService,
        MatIconRegistry,
        AlerttableService,
        EventsService,
        UIConfigsService,
        SearchService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ClusterComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('cluster should have right values', () => {
    component.cluster = new ClusterCluster({ 'kind': 'Cluster', 'api-version': 'v1', 'meta': { 'name': 'testCluster', 'tenant': 'default', 'namespace': 'default', 'resource-version': '454', 'uuid': '88f8baa7-7a8e-4a2a-9780-c23d2a217b5d', 'creation-time': '2018-06-13T17:50:29.117538562Z', 'mod-time': '2018-06-13T17:50:44.783819163Z', 'self-link': '/v1/cluster/cluster/testCluster' }, 'spec': { 'quorum-nodes': ['node1', 'node2', 'node3'], 'virtual-ip': '192.168.30.10', 'ntp-servers': ['1.pool.ntp.org', '2.pool.ntp.org'], 'auto-admit-nics': true }, 'status': { 'leader': 'node1', 'last-leader-transition-time': '2018-06-13T17:50:44.783544086Z' } });
    fixture.detectChanges();
    // Cluster icon should be visible
    const clusterDe: DebugElement = fixture.debugElement;
    const clusterHe: HTMLElement = clusterDe.nativeElement;
    expect(clusterDe.queryAll(By.css('.pagebody-icon')).length).toBe(1);
    expect(clusterHe.querySelector('div.pagebody-headercontent').textContent).toContain('testCluster');
    expect(clusterHe.querySelector('div.cluster-details-panel-leader-value').textContent).toContain('node1');
    expect(clusterHe.querySelector('div.cluster-details-panel-last-leader-transition-time').textContent).toContain('06-13-2018');
    expect(clusterHe.querySelector('div.cluster-details-panel-quorum-nodes').textContent).toContain('node1, node2, node3');
    expect(clusterHe.querySelector('div.cluster-details-panel-ntp-servers').textContent).toContain('1.pool.ntp.org, 2.pool.ntp.org');
    expect(clusterHe.querySelector('div.cluster-details-panel-virtual-ip').textContent).toContain('192.168.30.10');
    expect(clusterHe.querySelector('div.cluster-details-panel-auto-admit-nics').textContent).toContain('yes');
  });

  it('nodes should have right values', () => {
    const node1 = new ClusterNode({ 'kind': 'Node', 'api-version': 'v1', 'meta': { 'name': 'node1', 'tenant': 'default', 'namespace': 'default', 'resource-version': '5', 'uuid': '6a000b60-949b-4338-bd7e-8e750a9a8edb', 'creation-time': '2018-06-13T17:50:29.117624431Z', 'mod-time': '2018-06-13T17:50:29.117624431Z', 'self-link': '/v1/cluster/nodes/node1' }, 'spec': {}, 'status': { 'phase': 'JOINED', 'quorum': true } });
    const node2 = new ClusterNode({ 'kind': 'Node', 'api-version': 'v1', 'meta': { 'name': 'node2', 'tenant': 'default', 'namespace': 'default', 'resource-version': '5', 'uuid': '1d96fc5c-7701-4d8d-ada8-3ec160a50138', 'creation-time': '2018-06-13T17:50:29.117702613Z', 'mod-time': '2018-06-13T17:50:29.117702613Z', 'self-link': '/v1/cluster/nodes/node2' }, 'spec': {}, 'status': { 'phase': 'JOINED', 'quorum': true } });
    const node3 = new ClusterNode({ 'kind': 'Node', 'api-version': 'v1', 'meta': { 'name': 'node3', 'tenant': 'default', 'namespace': 'default', 'resource-version': '5', 'uuid': 'cdf2c24c-3afb-417f-a615-6a4cbd77f2c4', 'creation-time': '2018-06-13T17:50:29.117712981Z', 'mod-time': '2018-06-13T17:50:29.117712981Z', 'self-link': '/v1/cluster/nodes/node3' }, 'spec': {}, 'status': { 'phase': 'JOINED', 'quorum': true } });
    const node4 = new ClusterNode({ 'kind': 'Node', 'api-version': 'v1', 'meta': { 'name': 'node4', 'tenant': 'default', 'resource-version': '6918', 'uuid': 'ae07bb01-b50c-4941-9be8-133766b3725e', 'creation-time': '2018-06-13T18:22:28.189909722Z', 'mod-time': '2018-06-13T18:22:28.25393029Z', 'self-link': '/v1/cluster/nodes/node4' }, 'spec': {}, 'status': { 'phase': 'JOINED' } });
    const node5 = new ClusterNode({ 'kind': 'Node', 'api-version': 'v1', 'meta': { 'name': 'node5', 'tenant': 'default', 'resource-version': '6957', 'uuid': '199e7e27-a191-4332-9e28-e3c1f4445ddf', 'creation-time': '2018-06-13T18:22:30.542580696Z', 'mod-time': '2018-06-13T18:22:30.607773739Z', 'self-link': '/v1/cluster/nodes/node5' }, 'spec': {}, 'status': { 'phase': 'JOINED' } });
    component.nodes = [node1, node2, node3, node4, node5];
    fixture.detectChanges();
    const nodesDe: DebugElement = fixture.debugElement;
    const nodesHe: HTMLElement = nodesDe.nativeElement;
    // Table should be there
    const table: HTMLElement = nodesHe.querySelector('div.ui-table-wrapper');
    expect(table).toBeDefined();
    const rows = table.querySelectorAll('table tbody tr');
    expect(rows.length).toBe(5);
    for (let i = 0; i < 5; i++) {
      const cols = rows[i].querySelectorAll('td');
      expect(cols.length).toBe(4);
      expect(cols[1].textContent).toContain('node' + (i + 1));
      if (i < 3) {
        expect(cols[2].textContent).toContain('yes');
      } else {
        expect(cols[2].textContent).toBe('');
      }
      expect(cols[3].textContent).toContain('Joined');
    }
  });
});
