/**
 * These tests work on mac but do not
 * currently work on linux, due to issues with
 * the Jasmine testing framework in linux
 * evironments. There is expected to be a patch soon,
 * and we will fix this test once the patch is released.
 * - Jeff and Rohan

import { async, ComponentFixture, TestBed, inject } from '@angular/core/testing';
import { WidgetsModule } from 'web-app-framework';
import { ControllerService } from '@app/services/controller.service';
import { WorkloadService } from '@app/services/workload.service';

import { Observable } from 'rxjs/Observable';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClient } from '@angular/common/http';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { MockDataUtil } from '@app/common/MockDataUtil';

import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared//shared.module';
import { WorkloadwidgetComponent } from './workloadwidget.component';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { Component } from '@angular/core';

@Component( {
  template: ''
})
class DummyComponent { }

class MockWorkloadService extends WorkloadService {
  public getStaticWidgetDataById(id) {
    const data = {
      title: 'Total Workloads',
      icon: {
        margin : {
          top : '6px',
          right : '5px'
        },
        url : this.svgMap['totalworkloads']
      }
    };
    return data;
  }

  public getWidgetDataById(id) {
    const data = MockDataUtil.mockWorkloadWidget(8, 2, 400);
    return Observable.of(data);
  }
}
describe('WorkloadwidgetComponent', () => {
  let component: WorkloadwidgetComponent;
  let fixture: ComponentFixture<WorkloadwidgetComponent>;

  let testBedService: WorkloadService;
  let componentService: WorkloadService;
  let spy: any;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ WorkloadwidgetComponent, DummyComponent],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        WidgetsModule,
        MaterialdesignModule,
        SharedModule,
        BrowserAnimationsModule,
        HttpClientTestingModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        { provide: WorkloadService, useClass: MockWorkloadService }
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(WorkloadwidgetComponent);
    component = fixture.componentInstance;
    component.id = 'test_id';
    component.isPinned = false;
    component.widgetHover = false;
    component._iconPinnedStyles = {};
    component._iconStyles = {};

    fixture.detectChanges();

    // WorkloadService provided to the TestBed.overrideComponent(..)
    testBedService = TestBed.get(WorkloadService);

    // WorkloadService provided by Component, (should return MockAuthService)
    componentService = fixture.debugElement.injector.get(WorkloadService);
  });

  afterEach(() => {
    testBedService = null;
    componentService = null;
    component = null;
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('Service injected via inject(...) and TestBed.get(...) should be the same instance',
      inject([WorkloadService], (injectService: WorkloadService) => {
        expect(injectService).toBe(testBedService);
      })
  );

  it('Service injected via component should be and instance of MockAuthService', () => {
    expect(componentService instanceof MockWorkloadService).toBeTruthy();
  });

  it('should call getWidgetData()', () => {
    spy = spyOn(componentService, 'getWidgetDataById').and.callThrough();
    spyOn(componentService, 'getStaticWidgetDataById');
    component.getWidgetData();  // invoke the call component API.
    expect(component.data).toBeDefined();

    expect(componentService.getStaticWidgetDataById).toHaveBeenCalled();
    expect(componentService.getWidgetDataById).toHaveBeenCalled();
  });
});

*/
