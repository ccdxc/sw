/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { Component } from '@angular/core';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { FormsModule } from '@angular/forms';


import { MatIconRegistry } from '@angular/material';


/**-----
 Third-parties imports
 ------------------*/
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { AlerttableService } from 'app/services/alerttable.service';
import { AlerttableComponent } from './alerttable.component';
@Component({
  template: ''
})
class DummyComponent { }

describe('AlerttableComponent', () => {
  let component: AlerttableComponent;
  let fixture: ComponentFixture<AlerttableComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [AlerttableComponent, DummyComponent],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        MatIconRegistry,
        AlerttableService,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlerttableComponent);
    component = fixture.componentInstance;
    component.cols = [
      { field: 'id', header: 'ID' },
      { field: 'date', header: 'Date' },
      { field: 'name', header: 'Name' },
      { field: 'message', header: 'Message' },
      { field: 'severity', header: 'Severity' }
    ];

    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
