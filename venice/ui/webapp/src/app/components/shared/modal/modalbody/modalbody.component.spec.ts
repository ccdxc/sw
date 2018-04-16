import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ModalbodyComponent } from './modalbody.component';

describe('ModalbodyComponent', () => {
  let component: ModalbodyComponent;
  let fixture: ComponentFixture<ModalbodyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ModalbodyComponent ],
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalbodyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
