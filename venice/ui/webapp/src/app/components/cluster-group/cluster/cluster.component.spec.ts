/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component } from '@angular/core';

/**-----
 Venice UI -  imports
 ------------------*/
import { ClusterComponent } from './cluster.component';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/cluster.service';

/**-----
 Third-parties imports
 ------------------*/
 import { MatIconRegistry } from '@angular/material';

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
        HttpClientTestingModule
      ],
      providers: [
        ControllerService,
        ClusterService,
        MatIconRegistry
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ClusterComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
