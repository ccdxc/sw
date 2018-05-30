import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component } from '@angular/core';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { DsbdwidgetheaderComponent } from '@app/components/shared/dsbdwidgetheader/dsbdwidgetheader.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { ControllerService } from '@app/services/controller.service';
import { WorkloadService } from '@app/services/workload.service';
import { WidgetsModule } from 'web-app-framework';

import { DsbdworkloadComponent } from './dsbdworkload.component';

@Component({
  template: ''
})
class DummyComponent { }

describe('DsbdworkloadComponent', () => {
  let component: DsbdworkloadComponent;
  let fixture: ComponentFixture<DsbdworkloadComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [DsbdworkloadComponent, DsbdwidgetheaderComponent, DummyComponent],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule,
        HttpClientTestingModule
      ],
      providers: [
        ControllerService,
        WorkloadService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DsbdworkloadComponent);
    component = fixture.componentInstance;
    component.id = 'test_id';
    component.background_img = {
      url: 'test.com'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
