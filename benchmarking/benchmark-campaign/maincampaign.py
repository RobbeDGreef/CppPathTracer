import pathlib
from typing import Any, Dict, Iterable, List, Optional

from benchkit.benchmark import Benchmark, CommandAttachment, PostRunHook, PreRunHook
from benchkit.campaign import CampaignCartesianProduct, Constants, CampaignSuite
from benchkit.commandwrappers import CommandWrapper
from benchkit.commandwrappers.strace import StraceWrap
from benchkit.commandwrappers.env import EnvWrap
from benchkit.commandwrappers.perf import PerfReportWrap, PerfStatWrap
from benchkit.dependencies.packages import PackageDependency
from benchkit.platforms import Platform
from benchkit.sharedlibs import SharedLib
from benchkit.utils.types import CpuOrder, PathType, Environment    
from benchkit.platforms import get_remote_platform

FLAMEGRAPH_PATH = "/home/robbe/opt/FlameGraph"
PRESET_SCENE = "fast_cornell_benchmark"

class RayTracerBenchmark(Benchmark):
    """Benchmark object for main benchmark."""

    def __init__(
        self,
        src_dir: PathType,
        build_dir: PathType | None = None,
        command_wrappers: Iterable[CommandWrapper] = [],
        command_attachments: Iterable[CommandAttachment] = [],
        shared_libs: Iterable[SharedLib] = [],
        pre_run_hooks: Iterable[PreRunHook] = [],
        post_run_hooks: Iterable[PostRunHook] = [],
        platform: Platform = None
    ) -> None:
        super().__init__(
            command_wrappers=command_wrappers,
            command_attachments=command_attachments,
            shared_libs=shared_libs,
            pre_run_hooks=pre_run_hooks,
            post_run_hooks=post_run_hooks,
        )
        self._build_dir = build_dir
        self._bench_src_path = src_dir
        if platform is not None:
            self.platform = platform

    @property
    def bench_src_path(self) -> pathlib.Path:
        return self._bench_src_path

    @staticmethod
    def get_build_var_names() -> List[str]:
        return []

    @staticmethod
    def get_run_var_names() -> List[str]:
        return [
            "nb_threads",
            "preset",
        ]

    @staticmethod
    def get_tilt_var_names() -> List[str]:
        return []

    @staticmethod
    def _parse_results(
        output: str,
        nb_threads: int,
    ) -> Dict[str, str]:
        return {}

    def dependencies(self) -> List[PackageDependency]:
        return super().dependencies() + [
            PackageDependency("build-essential"),
            PackageDependency("cmake"),
        ]

    def prebuild_bench(self, **kwargs):
        pass

    def build_bench(self, **_kwargs) -> None:
        self.platform.comm.shell(
            command=f"./build.sh",
            current_dir=self._bench_src_path,
        )

    def clean_bench(self) -> None:
        pass

    def single_run(  # pylint: disable=arguments-differ
        self,
        preset: str,
        nb_threads: int = 2,
        **kwargs,
    ) -> str:

        threads = f"--threads {nb_threads}"
        preset = f"--preset {preset}"

        run_command = [
            "./raytracer",
            threads,
            preset,
        ]

        wrapped_run_command, wrapped_environment = self._wrap_command(
            run_command=run_command,
            environment={},
            **kwargs,
        )

        output = self.run_bench_command(
            environment={},
            run_command=run_command,
            wrapped_run_command=wrapped_run_command,
            current_dir=self._bench_src_path,
            wrapped_environment=wrapped_environment,
            print_output=False,
        )
        return output

    def parse_output_to_results(  # pylint: disable=arguments-differ
        self,
        command_output: str,
        run_variables: Dict[str, Any],
        **_kwargs,
    ) -> Dict[str, Any]:
        nb_threads = int(run_variables["nb_threads"])
        duration = command_output.splitlines()[-1].split(": ")[1].split()[0]
        return {"nb_threads": nb_threads, "duration": duration}


def create_campaign(nb_threads: list[int], preset_scene: str, nb_runs: int, platform: Platform | None = None):
    # The variables that have to be iterated through for the benchmark
    variables = {
        "nb_threads": nb_threads,
        "preset": [preset_scene],
    }

    perfstat_wrapper = PerfStatWrap(freq=1000, separator=";", events=["cache-misses"])
    wrapper = PerfReportWrap(flamegraph_path=FLAMEGRAPH_PATH, freq=1000)

    benchmark = RayTracerBenchmark(src_dir='../../', build_dir='.', command_wrappers=[
        wrapper,
        perfstat_wrapper,
    ], 
    platform=platform,
    post_run_hooks=[
        wrapper.post_run_hook_flamegraph,
        perfstat_wrapper.post_run_hook_update_results,
    ])

    return CampaignCartesianProduct(
            name="Raytracer benchmark",
            benchmark=benchmark,
            nb_runs=nb_runs,
            variables=variables,
            gdb=False,
            debug=False,
            constants=None,
            enable_data_dir=True
    )


platform = get_remote_platform("kot-robbe.ddns.net:2222")

# rsync source code to the remote



campaign = create_campaign(nb_threads=[8, 16], preset_scene=PRESET_SCENE, nb_runs=5, platform=platform)
campaigns = [campaign]

suite = CampaignSuite(campaigns=campaigns)
suite.print_durations()
suite.run_suite()
